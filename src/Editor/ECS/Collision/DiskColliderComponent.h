#pragma once

#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "ColliderComponent.h"

class DiskColliderComponent : public ColliderComponent
{
public:
	DiskColliderComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName = "DiskColliderComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	float getRadius() const { return m_radius; }
	void setRadius(float radius) { m_radius= radius; }

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const override;

private:
	float m_radius;
};