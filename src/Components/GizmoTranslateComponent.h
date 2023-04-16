#pragma once

#include "ObjectSystemFwd.h"
#include "MikanComponent.h"

class GizmoTranslateComponent : public MikanComponent
{
public:
	GizmoTranslateComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void update() override;

	void setEnabled(bool bEnabled);

protected:
	BoxColliderComponentWeakPtr m_centerHandle;
	BoxColliderComponentWeakPtr m_xyHandle;
	BoxColliderComponentWeakPtr m_xzHandle;
	BoxColliderComponentWeakPtr m_yzHandle;
	BoxColliderComponentWeakPtr m_xAxisHandle;
	BoxColliderComponentWeakPtr m_yAxisHandle;
	BoxColliderComponentWeakPtr m_zAxisHandle;
};