#pragma once

#include "ObjectSystemFwd.h"
#include "ComponentFwd.h"
#include "GizmoFwd.h"
#include "SceneComponent.h"

class GizmoTransformComponent : public SceneComponent
{
public:
	GizmoTransformComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

protected:
	GizmoTranslateComponentWeakPtr m_translateComponent;
	GizmoRotateComponentWeakPtr m_rotateComponent;
	GizmoScaleComponentWeakPtr m_scaleComponent;
};