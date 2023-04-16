#pragma once

#include "ObjectSystemFwd.h"
#include "MikanComponent.h"

class GizmoRotateComponent : public MikanComponent
{
public:
	GizmoRotateComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void update() override;

	void setEnabled(bool bEnabled);

protected:
	DiskColliderComponentWeakPtr m_xAxisHandle;
	DiskColliderComponentWeakPtr m_yAxisHandle;
	DiskColliderComponentWeakPtr m_zAxisHandle;
};