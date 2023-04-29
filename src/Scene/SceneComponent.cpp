#include "SceneComponent.h"
#include "MikanObject.h"
#include "MathGLM.h"

#include <glm/gtx/matrix_decompose.hpp>

SceneComponent::SceneComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

void SceneComponent::init()
{
	MikanComponent::init();
}

void SceneComponent::dispose()
{
	detachFromParent();
	MikanComponent::dispose();
}

void SceneComponent::attachToComponent(SceneComponentPtr newParentComponent)
{
	detachFromParent();

	SceneComponentPtr parent = m_parentComponent.lock();
	if (parent != nullptr)
	{
		SceneComponentList& parentChildList= parent->m_childComponents;

		parentChildList.push_back(SceneComponentPtr(this));
		m_parentComponent = newParentComponent;

		// Refresh world transform
		setRelativeTransform(m_relativeTransform);
	}
}

void SceneComponent::detachFromParent()
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
}

void SceneComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	m_relativeTransform= newRelativeXform;

	SceneComponentPtr parent = m_parentComponent.lock();
	if (parent != nullptr)
	{
		m_worldTransform= glm_composite_xform(parent->getWorldTransform(), newRelativeXform.getMat4());
	}
	else
	{
		m_worldTransform= newRelativeXform.getMat4();
	}

	if (m_renderable != nullptr)
	{
		m_renderable->setModelMatrix(m_worldTransform);
	}

	if (OnTranformChaged)
		OnTranformChaged(getSelfPtr<SceneComponent>());
}

void SceneComponent::setWorldTransform(const glm::mat4& newWorldXform)
{
	m_worldTransform= newWorldXform;

	if (m_renderable != nullptr)
	{
		m_renderable->setModelMatrix(m_worldTransform);
	}
	
	glm::mat4 invParentXform= glm::mat4(1.f);
	SceneComponentPtr parent = m_parentComponent.lock();
	if (parent != nullptr)
	{
		invParentXform= glm::inverse(parent->getWorldTransform());
	}

	const glm::mat4 relativeXform= glm_composite_xform(invParentXform, m_worldTransform);
	m_relativeTransform= GlmTransform(relativeXform);

	if (OnTranformChaged)
		OnTranformChaged(getSelfPtr<SceneComponent>());
}