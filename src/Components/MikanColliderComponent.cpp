#include "MikanColliderComponent.h"

MikanColliderComponent::MikanColliderComponent(MikanObjectWeakPtr owner)
	: MikanSceneComponent(owner)
{
}

bool MikanColliderComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	outResult.hitLocation = glm::vec3(0.f);
	outResult.hitNormal = glm::vec3(0.f);
	outResult.hitDistance = -1.f;

	return false;
}