#include "ColliderComponent.h"

ColliderComponent::ColliderComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
}

void ColliderComponent::setEnabled(bool bEnabled)
{
	m_bEnabled= bEnabled;
}

bool ColliderComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	outResult.hitValid= false;
	outResult.hitLocation = glm::vec3(0.f);
	outResult.hitNormal = glm::vec3(0.f);
	outResult.hitDistance = -1.f;
	outResult.hitComponent.reset();

	return false;
}