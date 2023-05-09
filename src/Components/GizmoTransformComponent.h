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

class GizmoTransformComponent : public SceneComponent
{
public:
	GizmoTransformComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	void bindInput();

	inline eGizmoMode getGizmoMode() const { return m_gizmoMode; }
	void setGizmoMode(eGizmoMode newMode);
	void selectTranslateMode();
	void selectRotateMode();
	void selectScaleMode();

	SceneComponentPtr getTransformTarget() const;
	void setTransformTarget(SceneComponentPtr sceneComponentTarget);
	void clearTransformTarget();
	void applyTransformToTarget();

protected:
	eGizmoMode m_gizmoMode= eGizmoMode::none;
	GizmoTranslateComponentWeakPtr m_translateComponent;
	GizmoRotateComponentWeakPtr m_rotateComponent;
	GizmoScaleComponentWeakPtr m_scaleComponent;
	SceneComponentWeakPtr m_targetSceneComponent;
};