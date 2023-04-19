#pragma once

#include "ColliderQuery.h"
#include "ObjectSystemFwd.h"
#include "MikanComponent.h"
#include "SinglecastDelegate.h"

#include <glm/ext/vector_float3.hpp>

class GizmoTranslateComponent : public MikanComponent
{
public:
	GizmoTranslateComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void update() override;
	virtual void dispose() override;

	void setEnabled(bool bEnabled);

	SinglecastDelegate<void(const glm::vec3& translation)> OnTranslationRequested;

protected:
	glm::vec3 getColliderColor(BoxColliderComponentWeakPtr colliderPtr, const glm::vec3& defaultColor);

	void onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult);
	void onInteractionGrab(const ColliderRaycastHitResult& hitResult);
	void onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onInteractionRelease();

	void requestTranslation(const glm::vec3& worldSpaceTranslation);
	
	bool m_bEnabled= false;
	BoxColliderComponentWeakPtr m_centerHandle;
	BoxColliderComponentWeakPtr m_xyHandle;
	BoxColliderComponentWeakPtr m_xzHandle;
	BoxColliderComponentWeakPtr m_yzHandle;
	BoxColliderComponentWeakPtr m_xAxisHandle;
	BoxColliderComponentWeakPtr m_yAxisHandle;
	BoxColliderComponentWeakPtr m_zAxisHandle;

	SelectionComponentWeakPtr m_selectionComponent;
	ColliderComponentWeakPtr m_hoverComponent;
	ColliderComponentWeakPtr m_dragComponent;
	glm::vec3 m_dragOrigin;
};