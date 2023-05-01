#pragma once

#include "ColliderQuery.h"
#include "ObjectSystemFwd.h"
#include "MikanComponent.h"
#include "SinglecastDelegate.h"

#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/vector_float3.hpp>

class GizmoRotateComponent : public MikanComponent
{
public:
	GizmoRotateComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;
	virtual void dispose() override;

	void setEnabled(bool bEnabled);

	SinglecastDelegate<void(const glm::quat& objectSpaceRotation)> OnRotateRequested;

protected:
	glm::vec3 getColliderColor(DiskColliderComponentWeakPtr colliderPtr, const glm::vec3& defaultColor) const;
	bool getColliderRotationAxis(
		ColliderComponentWeakPtr colliderWeakPtr,
		glm::vec3& outOrigin,
		glm::vec3& outAxis);

	void onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult);
	void onInteractionGrab(const ColliderRaycastHitResult& hitResult);
	void onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onInteractionRelease();

	void requestRotation(const glm::quat& objectSpaceRotation);

	bool m_bEnabled= false;
	DiskColliderComponentWeakPtr m_xAxisHandle;
	DiskColliderComponentWeakPtr m_yAxisHandle;
	DiskColliderComponentWeakPtr m_zAxisHandle;

	SelectionComponentWeakPtr m_selectionComponent;
	ColliderComponentWeakPtr m_hoverComponent;
	ColliderComponentWeakPtr m_dragComponent;
	glm::mat4 m_dragBasis;
	glm::vec3 m_dragStart;
	float m_dragAngle= 0.f;
};