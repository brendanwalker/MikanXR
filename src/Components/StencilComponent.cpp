#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "StencilComponent.h"
#include "SceneComponent.h"
#include "MikanObject.h"

StencilComponent::StencilComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

void StencilComponent::init()
{
	MikanComponent::init();

	SceneComponentPtr sceneComponentPtr= getOwnerObject()->getRootComponent();
	sceneComponentPtr->OnTranformChaged += MakeDelegate(this, &StencilComponent::onSceneComponentTranformChaged);
	m_sceneComponent= sceneComponentPtr;
}

void StencilComponent::dispose()
{
	SceneComponentPtr sceneComponentPtr = m_sceneComponent.lock();
	sceneComponentPtr->OnTranformChaged -= MakeDelegate(this, &StencilComponent::onSceneComponentTranformChaged);

	MikanComponent::dispose();
}

glm::mat4 StencilComponent::getStencilLocalTransform() const
{
	return m_sceneComponent.lock()->getRelativeTransform().getMat4();
}

glm::mat4 StencilComponent::getStencilWorldTransform() const
{
	return m_sceneComponent.lock()->getWorldTransform();
}

void StencilComponent::attachSceneComponentToAnchor(MikanSpatialAnchorID newParentId)
{
	SceneComponentPtr sceneComponent = m_sceneComponent.lock();

	if (sceneComponent != nullptr)
	{
		if (newParentId != INVALID_MIKAN_ID)
		{
			AnchorComponentPtr anchor = AnchorObjectSystem::getSystem()->getSpatialAnchorById(newParentId);

			if (anchor)
			{
				sceneComponent->attachToComponent(anchor->getOwnerObject()->getRootComponent());
			}
			else
			{
				sceneComponent->detachFromParent();
			}
		}
		else
		{
			sceneComponent->detachFromParent();
		}
	}
}

void StencilComponent::onSceneComponentTranformChaged(SceneComponentPtr sceneComponentPtr)
{
	if (!m_bUpdatingSceneComponentTransform)
	{
		setConfigTransform(sceneComponentPtr->getRelativeTransform());
	}
}

void StencilComponent::applyConfigTransformToSceneComponent()
{
	SceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		m_bUpdatingSceneComponentTransform = true;
		sceneComponent->setRelativeTransform(getConfigTransform());
		m_bUpdatingSceneComponentTransform = false;
	}
}