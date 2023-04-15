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
	return glm_intersect_disk_with_ray(
		request.rayOrigin,
		request.rayDirection,
		m_worldTransform[3], // center
		m_worldTransform[1], // y-axis
		m_radius,
		outResult.hitDistance,
		outResult.hitLocation,
		outResult.hitNormal);
}