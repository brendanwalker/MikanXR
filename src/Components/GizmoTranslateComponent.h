#pragma once

#include "ColliderQuery.h"
#include "ObjectSystemFwd.h"
#include "MikanComponent.h"

class GizmoTranslateComponent : public MikanComponent
{
public:
	GizmoTranslateComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void update() override;
	virtual void dispose() override;

	void setEnabled(bool bEnabled);

protected:
	void onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayPress(const ColliderRaycastHitResult& hitResult, int button);
	void onInteractionRayRelease(const ColliderRaycastHitResult& hitResult, int button);
	
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
};