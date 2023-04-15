#pragma once

#include "ObjectSystemFwd.h"
#include "MikanComponent.h"

class GizmoScaleComponent : public MikanComponent
{
public:
	GizmoScaleComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void update() override;

protected:
	BoxColliderComponentWeakPtr m_xAxisHandle;
	BoxColliderComponentWeakPtr m_yAxisHandle;
	BoxColliderComponentWeakPtr m_zAxisHandle;
	BoxColliderComponentWeakPtr m_centerHandle;
};