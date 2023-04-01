#include "MikanSceneComponent.h"
#include "MikanObject.h"
#include "MathGLM.h"

#include <glm/gtx/matrix_decompose.hpp>

MikanSceneComponent::MikanSceneComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

void MikanSceneComponent::init()
{
	MikanComponent::init();
}

void MikanSceneComponent::dispose()
{
	detachFromParent();
	MikanComponent::dispose();
}

void MikanSceneComponent::attachToComponent(MikanSceneComponentWeakPtr newParentComponent)
{
	detachFromParent();

	MikanSceneComponentPtr parent = m_parentComponent.lock();
	if (parent != nullptr)
	{
		MikanSceneComponentList& parentChildList= parent->m_childComponents;

		parentChildList.push_back(MikanSceneComponentPtr(this));
		m_parentComponent = newParentComponent;

		// Refresh world transform
		setRelativeTransform(m_relativeTransform);
	}
}

void MikanSceneComponent::detachFromParent()
{
	MikanSceneComponentPtr parent= m_parentComponent.lock();
	if (parent != nullptr)
	{
		MikanSceneComponentList& parentChildList= parent->m_childComponents;

		for (auto it = parentChildList.begin(); it != parentChildList.end(); ++it)
		{
			MikanSceneComponentPtr sceneComponent= it->lock(); 

			if (sceneComponent.get() == this)
			{
				parentChildList.erase(it);
				break;
			}
		}
	}

	m_parentComponent = MikanSceneComponentPtr();
	m_worldTransform = m_relativeTransform.getMat4();
}

void MikanSceneComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	m_relativeTransform= newRelativeXform;

	MikanSceneComponentPtr parent = m_parentComponent.lock();
	if (parent != nullptr)
	{
		m_worldTransform= glm_composite_xform(parent->getWorldTransform(), newRelativeXform.getMat4());
	}
	else
	{
		m_worldTransform= newRelativeXform.getMat4();
	}
}

void MikanSceneComponent::setWorldTransform(const glm::mat4& newWorldXform)
{
	m_worldTransform= newWorldXform;
	
	glm::mat4 invParentXform= glm::mat4(1.f);
	MikanSceneComponentPtr parent = m_parentComponent.lock();
	if (parent != nullptr)
	{
		invParentXform= glm::inverse(parent->getWorldTransform());
	}

	const glm::mat4 relativeXform= glm_composite_xform(invParentXform, m_worldTransform);
	glm::vec3 scale;
	glm::quat orientation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	if (glm::decompose(
		relativeXform,
		scale, orientation, translation, skew, perspective))
	{
		m_relativeTransform= GlmTransform(translation, orientation, scale);
	}
	else
	{
		m_relativeTransform= GlmTransform();
	}
}