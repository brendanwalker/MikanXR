#pragma once

#include "ColliderQuery.h"
#include "ObjectSystemFwd.h"
#include "MikanComponent.h"
#include "SinglecastDelegate.h"

#include <glm/ext/vector_float3.hpp>

class GizmoScaleComponent : public MikanComponent
{
public:
	GizmoScaleComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;
	virtual void dispose() override;

	void setEnabled(bool bEnabled);

	SinglecastDelegate<void(const glm::vec3& objectSpaceScale)> OnScaleRequested;

protected:
	glm::vec3 getColliderColor(BoxColliderComponentWeakPtr colliderPtr, const glm::vec3& defaultColor) const;

	void onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult);
	void onInteractionGrab(const ColliderRaycastHitResult& hitResult);
	void onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onInteractionRelease();

	void requestScale(const glm::vec3& objectSpaceScale);

	bool m_bEnabled= false;
	BoxColliderComponentWeakPtr m_xAxisHandle;
	BoxColliderComponentWeakPtr m_yAxisHandle;
	BoxColliderComponentWeakPtr m_zAxisHandle;
	BoxColliderComponentWeakPtr m_centerHandle;

	SelectionComponentWeakPtr m_selectionComponent;
	ColliderComponentWeakPtr m_hoverComponent;
	ColliderComponentWeakPtr m_dragComponent;
	glm::vec3 m_dragOrigin;
};