#include "App.h"
#include "ColliderComponent.h"
#include "MikanObject.h"
#include "SelectionComponent.h"
#include "ObjectSystemManager.h"

SelectionComponent::SelectionComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

void SelectionComponent::init()
{
	MikanComponent::init();

	rebindColliders();
}

void SelectionComponent::dispose()
{
	m_colliders.clear();

	MikanComponent::dispose();
}

void SelectionComponent::rebindColliders()
{
	getOwnerObject()->getComponentsOfWeakType<ColliderComponent>(m_colliders);
}

bool SelectionComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	bool bAnyHits= false;

	for (auto& colliderWeakPtr : m_colliders)
	{
		auto colliderPtr= colliderWeakPtr.lock();

		if (colliderPtr && colliderPtr->getEnabled())
		{
			ColliderRaycastHitResult result;
			if (colliderPtr->computeRayIntersection(request, result))
			{
				if (!bAnyHits || 
					result.hitDistance < outResult.hitDistance || 
					result.hitPriority > outResult.hitPriority)
				{
					outResult = result;
					bAnyHits = true;
				}
			}
		}
	}

	return bAnyHits;
}

void SelectionComponent::notifyHoverEnter(const ColliderRaycastHitResult& hitResult)
{
	m_bIsHovered= true;
	if (OnInteractionRayOverlapEnter)
		OnInteractionRayOverlapEnter(hitResult);
}

void SelectionComponent::notifyHoverExit(const ColliderRaycastHitResult& hitResult)
{
	m_bIsHovered= false;
	if (OnInteractionRayOverlapExit)
		OnInteractionRayOverlapExit(hitResult);
}

void SelectionComponent::notifyGrab(const ColliderRaycastHitResult& hitResult)
{
	m_bIsGrabbed= true;
	if (OnInteractionGrab)
		OnInteractionGrab(hitResult);
}

void SelectionComponent::notifyMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	if (OnInteractionMove)
		OnInteractionMove(rayOrigin, rayDir);
}

void SelectionComponent::notifyRelease()
{
	m_bIsGrabbed= false;
	if (OnInteractionRelease)
		OnInteractionRelease();
}

void SelectionComponent::notifySelected()
{
	m_bIsSelected= true;
	if (OnInteractionSelected)
		OnInteractionSelected();
}

void SelectionComponent::notifyUnselected()
{
	m_bIsSelected= false;
	if (OnInteractionUnselected)
		OnInteractionUnselected();
}

void SelectionComponent::notifyTransformGizmoBound()
{
	m_bIsTransformGizmoBound= true;
	if (OnTransformGizmoBound)
		OnTransformGizmoBound();
}

void SelectionComponent::notifyTransformGizmoUnbound()
{
	m_bIsTransformGizmoBound= true;
	if (OnTransformGizmoUnbound)
		OnTransformGizmoUnbound();
}
