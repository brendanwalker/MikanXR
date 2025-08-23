#include "DiskColliderComponent.h"
#include "MikanObject.h"
#include "MathGLM.h"

DiskColliderComponent::DiskColliderComponent(MikanObjectWeakPtr owner)
	: ColliderComponent(owner)
{}

bool DiskColliderComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	outResult.hitValid = false;
	outResult.hitLocation = glm::vec3(0.f);
	outResult.hitNormal = glm::vec3(0.f);
	outResult.hitDistance = -1.f;
	outResult.hitPriority = m_priority;
	outResult.hitComponent.reset();

	if (!m_bEnabled)
		return false;

	outResult.hitValid=
		glm_intersect_disk_with_ray(
			request.rayOrigin,
			request.rayDirection,
			m_worldTransform[3], // center
			m_worldTransform[1], // y-axis
			m_radius,
			outResult.hitDistance,
			outResult.hitLocation,
			outResult.hitNormal);

	if (outResult.hitValid)
	{
		outResult.hitComponent =
			std::const_pointer_cast<ColliderComponent>(
				getSelfPtr<const ColliderComponent>());
	}

	return outResult.hitValid;
}