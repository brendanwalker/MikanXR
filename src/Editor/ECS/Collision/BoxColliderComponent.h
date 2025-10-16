#pragma once

#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "ColliderComponent.h"

#include <glm/ext/vector_float3.hpp>

class BoxColliderComponent : public ColliderComponent
{
public:
	BoxColliderComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName = "BoxColliderComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	const glm::vec3 getHalfExtents() const { return m_halfExtents; }
	void setHalfExtents(const glm::vec3 halfExtents) { m_halfExtents= halfExtents; }

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const override;

private:
	glm::vec3 m_halfExtents;
};