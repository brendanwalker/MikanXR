#pragma once

#include "ObjectSystemFwd.h"
#include "ComponentFwd.h"
#include "GizmoFwd.h"
#include "SceneComponent.h"

enum class eGizmoMode : int
{
	none,
	translate,
	rotate,
	scale
};

class GizmoTransformComponent : public SceneComponent, public IGlLineRenderable
{
public:
	GizmoTransformComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void renderLines() const override;
	virtual void dispose() override;

	inline eGizmoMode getGizmoMode() const { return m_gizmoMode; }
	void setGizmoMode(eGizmoMode newMode);
	void selectTranslateMode();
	void selectRotateMode();
	void selectScaleMode();

protected:
	eGizmoMode m_gizmoMode= eGizmoMode::none;
	GizmoTranslateComponentWeakPtr m_translateComponent;
	GizmoRotateComponentWeakPtr m_rotateComponent;
	GizmoScaleComponentWeakPtr m_scaleComponent;
};