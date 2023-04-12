#include "ColliderComponent.h"

ColliderComponent::ColliderComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
}

bool ColliderComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	outResult.hitLocation = glm::vec3(0.f);
	outResult.hitNormal = glm::vec3(0.f);
	outResult.hitDistance = -1.f;

	return false;
}