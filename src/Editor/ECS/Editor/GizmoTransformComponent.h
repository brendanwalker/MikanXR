#pragma once

#include "ObjectSystemFwd.h"
#include "ComponentFwd.h"
#include "GizmoFwd.h"
#include "TransformComponent.h"

enum class eGizmoMode : int
{
	none,
	translate,
	rotate,
	scale
};

class GizmoTransformComponent : public TransformComponent
{
public:
	GizmoTransformComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName = "GizmoTransformComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	static constexpr float k_gizmoBaseRadius = 0.5f;
	static constexpr float k_gizmoBaseWidth = 0.05f;
	static constexpr float k_gizmoScreenSizeFactor = 0.15f;

	virtual void init() override;
	virtual void update(float deltaSeconds) override;
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
	TransformComponentWeakPtr m_transformTarget;

	glm::vec3 m_targetScale;
	float m_displayScale = 1.0f;
	bool m_bIsApplyingTransformToTarget= false;
	float computeDisplayScale() const;
	void updateGizmoColliderScales();
	void applyTransformToTarget();
	void applyTransformToGizmo();

	void onSelectionTranslationRequested(const glm::vec3& worldSpaceTranslation);
	void onSelectionRotationRequested(const glm::quat& worldSpaceRotation);
	void onSelectionScaleRequested(const glm::vec3& objectSpaceScale);

	void onTransformTargetConfigChange(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
};