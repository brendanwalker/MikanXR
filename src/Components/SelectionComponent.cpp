#include "App.h"
#include "ColliderComponent.h"
#include "MikanObject.h"
#include "SelectionComponent.h"
#include "ObjectSystemManager.h"

SelectionComponent::SelectionComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

SelectionComponent::~SelectionComponent()
{
	dispose();
}

void SelectionComponent::init()
{
	getOwnerObject()->getComponentsOfType<ColliderComponent>(m_colliders);
}

void SelectionComponent::dispose()
{
	m_colliders.clear();
}

bool SelectionComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	bool bAnyHits= false;

	for (auto& colliderPtr : m_colliders)
	{
		ColliderRaycastHitResult result;
		if (colliderPtr->computeRayIntersection(request, result))
		{
			if (!bAnyHits || result.hitDistance < outResult.hitDistance)
			{
				outResult= result;
				bAnyHits= true;
			}
		}
	}

	return bAnyHits;
}