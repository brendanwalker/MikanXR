#pragma once

#include "ObjectSystemFwd.h"
#include "MikanComponent.h"

class GizmoScaleComponent : public MikanComponent
{
public:
	GizmoScaleComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void update() override;

	void setEnabled(bool bEnabled);

protected:
	bool m_bEnabled= false;
	BoxColliderComponentWeakPtr m_xAxisHandle;
	BoxColliderComponentWeakPtr m_yAxisHandle;
	BoxColliderComponentWeakPtr m_zAxisHandle;
	BoxColliderComponentWeakPtr m_centerHandle;
};