#include "ColliderQuery.h"
#include "ColliderComponent.h"
#include "MathUtility.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "ObjectSystemColliderQueries.h"

ColliderRaycastHitResult findClosestCollisionAlongRay(
	std::set<const MikanObjectSystem*> objectSystems,
	const ColliderRaycastHitRequest& request)
{
	ColliderRaycastHitResult closestResult = {};

	for (const auto* objectSystem : objectSystems)
	{
		if (objectSystem)
		{
			ColliderRaycastHitResult result =
				findClosestCollisionAlongRay(
					objectSystem->shared_from_this(), 
					request, 
					&closestResult);

			if (result.hitValid && result.isHigherPriorityThan(closestResult))
			{
				closestResult = result;
			}
		}
	}

	return closestResult;
}

ColliderRaycastHitResult findClosestCollisionAlongRay(
	MikanObjectSystemConstPtr objectSystem,
	const ColliderRaycastHitRequest& request,
	const ColliderRaycastHitResult* inPrevClosestResult)
{
	ColliderRaycastHitResult closestResult;

	if (inPrevClosestResult)
	{
		closestResult = *inPrevClosestResult;
	}
	else
	{
		closestResult = ColliderRaycastHitResult();
		closestResult.hitDistance = k_real_max;
		closestResult.hitPriority = 0;
	}

	objectSystem->visitAllObjects([&request, &closestResult](MikanObjectPtr objectPtr) {
		objectPtr->visitAllComponents([objectPtr, &request, &closestResult](MikanComponentPtr componentPtr) {
			auto colliderComponent = std::dynamic_pointer_cast<ColliderComponent>(componentPtr);
			if (colliderComponent)
			{
				ColliderRaycastHitResult colliderResult;

				if (colliderComponent->computeRayIntersection(request, colliderResult) &&
					colliderResult.isHigherPriorityThan(closestResult))
				{
					closestResult = colliderResult;
				}
			}
		});
	});

	return closestResult;
}
