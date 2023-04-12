#include "BoxColliderComponent.h"
#include "MikanObject.h"
#include "StaticMeshComponent.h"
#include "MulticastDelegate.h"
#include "GlStaticMeshInstance.h"

#include <glm/gtx/intersect.hpp>

BoxColliderComponent::BoxColliderComponent(MikanObjectWeakPtr owner)
	: ColliderComponent(owner)
{
}

bool BoxColliderComponent::computeRayIntersection(
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