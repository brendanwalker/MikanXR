#pragma once

#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "ColliderComponent.h"

class DiskColliderComponent : public ColliderComponent
{
public:
	DiskColliderComponent(MikanObjectWeakPtr owner);

	void setRadius(float radius) { m_radius= radius; }

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const override;

private:
	float m_radius;
};