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

	SelectionComponentPtr getSelectionTarget() const;
	void setSelectionTarget(SelectionComponentPtr selectionTarget);
	void clearSelectionTarget();

protected:
	eGizmoMode m_gizmoMode= eGizmoMode::none;
	GizmoTranslateComponentWeakPtr m_translateComponent;
	GizmoRotateComponentWeakPtr m_rotateComponent;
	GizmoScaleComponentWeakPtr m_scaleComponent;
	SelectionComponentWeakPtr m_selectionTarget;
	SceneComponentWeakPtr m_transformTarget;

	glm::vec3 m_targetScale;
	bool m_bIsApplyingTransformToTarget= false;
	void applyTransformToTarget();
	void applyTransformToGizmo();

	void onSelectionTranslationRequested(const glm::vec3& worldSpaceTranslation);
	void onSelectionRotationRequested(const glm::quat& worldSpaceRotation);
	void onSelectionScaleRequested(const glm::vec3& objectSpaceScale);

	void onTransformTargetConfigChange(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
};