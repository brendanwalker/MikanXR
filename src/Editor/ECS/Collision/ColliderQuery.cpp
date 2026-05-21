#include "ColliderQuery.h"
#include "MathUtility.h"

ColliderRaycastHitResult::ColliderRaycastHitResult()
{
	hitValid= false;
	hitDistance= k_real_max;
	hitPriority= -1;
	hitLocation= glm::vec3(0.f);
	hitNormal= glm::vec3(0.f);
	hitComponent.reset();
	closestVertexLocal= glm::vec3(0.f);
	closestVertexWorld= glm::vec3(0.f);
	closestVertexValid= false;
}

const bool ColliderRaycastHitResult::isHigherPriorityThan(const ColliderRaycastHitResult& other) const
{
	if (!this->hitValid)
		return false;

	if (!other.hitValid)
		return true;

	if (this->hitPriority < other.hitPriority)
		return false;

	if (this->hitPriority > other.hitPriority)
		return true;

	return this->hitDistance < other.hitDistance;
}
