#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "StencilComponent.h"
#include "SceneComponent.h"
#include "MikanObject.h"

StencilComponent::StencilComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
}

void StencilComponent::attachSceneComponentToAnchor(MikanSpatialAnchorID newParentId)
{
	if (newParentId != INVALID_MIKAN_ID)
	{
		AnchorComponentPtr anchor = AnchorObjectSystem::getSystem()->getSpatialAnchorById(newParentId);

		if (anchor)
		{
			attachToComponent(anchor->getOwnerObject()->getRootComponent());
		}
		else
		{
			detachFromParent(eDetachReason::detachFromParent);
		}
	}
	else
	{
		detachFromParent(eDetachReason::detachFromParent);
	}
}