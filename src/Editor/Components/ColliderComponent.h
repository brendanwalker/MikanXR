#pragma once

#include "ColliderQuery.h"
#include "ColliderComponent.h"
#include "SceneComponent.h"
#include "IMkMesh.h"
#include "ObjectFwd.h"

#include <memory>

class ColliderComponent : public SceneComponent
{
public:
	ColliderComponent(MikanObjectWeakPtr owner);

	inline bool getEnabled() const { return m_bEnabled; }
	void setEnabled(bool bEnabled);

	inline int getPriority() const { return m_priority; }
	void setPriority(int priority) { m_priority= priority; }

	virtual bool getBoundingSphere(glm::vec3 & outCenter, float& outRadius) const { return false; }
	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const;

protected:
	bool m_bEnabled= true;
	int m_priority= 0;
};