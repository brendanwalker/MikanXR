#include "BoxColliderComponent.h"
#include "MikanObject.h"
#include "MathGLM.h"

BoxColliderComponent::BoxColliderComponent(MikanObjectWeakPtr owner)
	: ColliderComponent(owner)
{
}

bool BoxColliderComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	outResult.hitValid=
		glm_intersect_obb_with_ray(
			request.rayOrigin,
			request.rayDirection,
			m_halfExtents,
			m_halfExtents*-1.f,
			getWorldTransform(),
			outResult.hitDistance,
			outResult.hitLocation,
			outResult.hitNormal);

	return outResult.hitValid;
}