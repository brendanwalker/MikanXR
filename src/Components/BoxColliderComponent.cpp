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
	outResult.hitValid = false;
	outResult.hitLocation = glm::vec3(0.f);
	outResult.hitNormal = glm::vec3(0.f);
	outResult.hitDistance = -1.f;
	outResult.hitComponent.reset();

	if (!m_bEnabled)
		return false;

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

	if (outResult.hitValid)
	{
		outResult.hitComponent= 
			std::const_pointer_cast<ColliderComponent>(
				getSelfPtr<const ColliderComponent>());
	}

	return outResult.hitValid;
}