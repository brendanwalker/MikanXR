#include "SceneComponent.h"
#include "MikanObject.h"
#include "MathGLM.h"

#include <glm/gtx/matrix_decompose.hpp>

SceneComponent::SceneComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, m_relativeTransform()
	, m_worldTransform(glm::mat4(1.f))
{
}

void SceneComponent::init()
{
	MikanComponent::init();
}

void SceneComponent::dispose()
{
	detachFromParent(true);
	MikanComponent::dispose();
}

void SceneComponent::attachToComponent(SceneComponentPtr newParentComponent)
{
	const bool bHasNewParent= newParentComponent != nullptr;

	// Don't bother propagating world transform changes if we have a new parent,
	// since the attachment to the new parent will propagate changes anyway
	const bool bUpdateChildWorldTransforms= !bHasNewParent;
	detachFromParent(bUpdateChildWorldTransforms);

	if (bHasNewParent)
	{
		SceneComponentList& parentChildList= newParentComponent->m_childComponents;

		parentChildList.push_back(getSelfWeakPtr<SceneComponent>());
		m_parentComponent = newParentComponent;

		// Refresh world transform
		setRelativeTransform(m_relativeTransform);
	}
}

void SceneComponent::detachFromParent(bool bUpdateChildWorldTransforms)
{
	SceneComponentPtr parent= m_parentComponent.lock();
	if (parent != nullptr)
	{
		SceneComponentList& parentChildList= parent->m_childComponents;

		for (auto it = parentChildList.begin(); it != parentChildList.end(); ++it)
		{
			SceneComponentPtr sceneComponent= it->lock(); 

			if (sceneComponent.get() == this)
			{
				parentChildList.erase(it);
				break;
			}
		}
	}

	m_parentComponent = SceneComponentPtr();

	// World transform is now the same as relative transform
	m_worldTransform= m_relativeTransform.getMat4();

	// Propagate world transform change to children (unless caller doesn't want us to)
	if (bUpdateChildWorldTransforms)
	{
		propogateWorldTransformChange(false);
	}
}

void SceneComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	// Set the new relative transform directly
	m_relativeTransform= newRelativeXform;

	propogateWorldTransformChange(true);
}

void SceneComponent::setWorldTransform(const glm::mat4& newWorldXform)
{
	// Set the new world transform directly
	m_worldTransform= newWorldXform;

	// Subtract our new world transform from out parent to compute new relative transform
	glm::mat4 invParentXform= glm::mat4(1.f);
	SceneComponentPtr parent = m_parentComponent.lock();
	if (parent != nullptr)
	{
		invParentXform= glm::inverse(parent->getWorldTransform());
	}
	const glm::mat4 relativeXform= glm_composite_xform(invParentXform, m_worldTransform);
	m_relativeTransform= GlmTransform(relativeXform);

	propogateWorldTransformChange(false);
}

const glm::vec3 SceneComponent::getWorldLocation() const
{
	return glm_mat4_get_position(m_worldTransform);
}

void SceneComponent::propogateWorldTransformChange(bool bRebuildWorldTransform)
{
	// Recompute our world transform, if requested
	if (bRebuildWorldTransform)
	{
		// Concat our transform to our parent's world transform
		SceneComponentPtr parent = m_parentComponent.lock();
		if (parent != nullptr)
		{
			m_worldTransform = glm_composite_xform(parent->getWorldTransform(), m_relativeTransform.getMat4());
		}
		else
		{
			m_worldTransform = m_relativeTransform.getMat4();
		}
	}

	// Update the world transform on the attached IGlSceneRenderable
	if (m_sceneRenderable != nullptr)
	{
		m_sceneRenderable->setModelMatrix(m_worldTransform);
	}

	// Broadcast to anyone that cares our transform changed
	if (OnTranformChaged)
		OnTranformChaged(getSelfPtr<SceneComponent>());

	// Propagate our updated world transform to our children
	for (SceneComponentWeakPtr childComponentWeakPtr : m_childComponents)
	{
		SceneComponentPtr childComponent= childComponentWeakPtr.lock();

		if (childComponent != nullptr)
		{
			childComponent->propogateWorldTransformChange(true);
		}
	}
}