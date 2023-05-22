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
	// Detach all children from this scene component first
	while (m_childComponents.size() > 0)
	{
		SceneComponentPtr childComponent= m_childComponents[0].lock();

		if (childComponent)
		{
			childComponent->detachFromParent(eDetachReason::parentDisposed);
		}
	}

	// Detach from our parent next
	if (m_parentComponent.lock())
	{
		detachFromParent(eDetachReason::selfDisposed);
	}

	MikanComponent::dispose();
}

bool SceneComponent::attachToComponent(SceneComponentPtr newParentComponent)
{
	SceneComponentPtr oldParentComponent= m_parentComponent.lock();

	if (!newParentComponent || newParentComponent == oldParentComponent)
		return false;

	// Detach from old parent
	if (oldParentComponent)
	{
		detachFromParent(eDetachReason::detachFromParent);
	}

	// Attach to new parent
	SceneComponentList& parentChildList= newParentComponent->m_childComponents;

	parentChildList.push_back(getSelfWeakPtr<SceneComponent>());
	m_parentComponent = newParentComponent;

	// Refresh world transform
	setRelativeTransform(m_relativeTransform);
}

void SceneComponent::detachFromParent(eDetachReason reason)
{
	// Remove ourselves from our current parent's child list
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

	// Forget about our parent
	m_parentComponent = SceneComponentPtr();

	// World transform is now the same as relative transform
	m_worldTransform= m_relativeTransform.getMat4();

	// Only bother propagating our new transform to our children 
	// if we just doing a simple detach from our current parent
	if (reason == eDetachReason::detachFromParent)
	{
		propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
	}
}

void SceneComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	// Set the new relative transform directly
	m_relativeTransform= newRelativeXform;

	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
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

	propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
}

const glm::vec3 SceneComponent::getWorldLocation() const
{
	return glm_mat4_get_position(m_worldTransform);
}

void SceneComponent::propogateWorldTransformChange(eTransformChangeType reason)
{
	// Recompute our world transform, if requested
	if (reason == eTransformChangeType::recomputeWorldTransformAndPropogate)
	{
		// Concat our transform to our parent's world transform
		SceneComponentPtr parent = m_parentComponent.lock();
		if (parent != nullptr)
		{
			m_worldTransform = glm_composite_xform(m_relativeTransform.getMat4(), parent->getWorldTransform());
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

	// Propagate our updated world transform to our children
	for (SceneComponentWeakPtr childComponentWeakPtr : m_childComponents)
	{
		SceneComponentPtr childComponent= childComponentWeakPtr.lock();

		if (childComponent != nullptr)
		{
			// Ask all children to recompute their world transform
			childComponent->propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
		}
	}
}