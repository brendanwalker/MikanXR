#include "MikanColliderComponent.h"

MikanColliderComponent::MikanColliderComponent(MikanObjectWeakPtr owner)
	: MikanSceneComponent(owner)
{
}

bool MikanColliderComponent::hasRayOverlap(
	const glm::vec3 origin, const glm::vec3 direction,
	ColliderRaycastHitResult& outHitResult) const
{
	return false;
}