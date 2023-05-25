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
			if (attachToComponent(anchor->getOwnerObject()->getRootComponent()))
			{
				onParentAnchorChanged(newParentId);
			}
		}
		else
		{
			detachFromParent(eDetachReason::detachFromParent);
			onParentAnchorChanged(INVALID_MIKAN_ID);
		}
	}
	else
	{
		detachFromParent(eDetachReason::detachFromParent);
		onParentAnchorChanged(INVALID_MIKAN_ID);
	}
}