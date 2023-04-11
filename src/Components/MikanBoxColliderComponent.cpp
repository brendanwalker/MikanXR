#include "MikanBoxColliderComponent.h"
#include "MikanObject.h"
#include "MikanStaticMeshComponent.h"
#include "MulticastDelegate.h"
#include "GlStaticMeshInstance.h"

#include <glm/gtx/intersect.hpp>

MikanBoxColliderComponent::MikanBoxColliderComponent(MikanObjectWeakPtr owner)
	: MikanColliderComponent(owner)
{
}

bool MikanBoxColliderComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	return glm_intersect_obb_with_ray(
		request.rayOrigin,
		request.rayDirection,
		m_halfExtents,
		m_halfExtents*-1.f,
		getWorldTransform(),
		outResult.hitDistance,
		outResult.hitLocation,
		outResult.hitNormal);
}