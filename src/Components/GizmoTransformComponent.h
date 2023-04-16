#pragma once

#include "ObjectSystemFwd.h"
#include "ComponentFwd.h"
#include "GizmoFwd.h"
#include "SceneComponent.h"

enum class eGizmoMode : int
{
	translate,
	rotate,
	scale
};

class GizmoTransformComponent : public SceneComponent
{
public:
	GizmoTransformComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

	inline eGizmoMode getGizmoMode() const { return m_gizmoMode; }
	void setGizmoMode(eGizmoMode newMode);

protected:
	eGizmoMode m_gizmoMode= eGizmoMode::translate;
	GizmoTranslateComponentWeakPtr m_translateComponent;
	GizmoRotateComponentWeakPtr m_rotateComponent;
	GizmoScaleComponentWeakPtr m_scaleComponent;
};