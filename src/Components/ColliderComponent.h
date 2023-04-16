#pragma once

#include "ColliderQuery.h"
#include "ColliderComponent.h"
#include "SceneComponent.h"
#include "IGlMesh.h"
#include "ObjectFwd.h"

#include <memory>

class ColliderComponent : public SceneComponent
{
public:
	ColliderComponent(MikanObjectWeakPtr owner);

	inline bool getEnabled() { return m_bEnabled; }
	void setEnabled(bool bEnabled);

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const;

protected:
	bool m_bEnabled= true;
};