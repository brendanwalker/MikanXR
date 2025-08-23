#pragma once

#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "ColliderComponent.h"

class DiskColliderComponent : public ColliderComponent
{
public:
	DiskColliderComponent(MikanObjectWeakPtr owner);

	float getRadius() const { return m_radius; }
	void setRadius(float radius) { m_radius= radius; }

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const override;

private:
	float m_radius;
};