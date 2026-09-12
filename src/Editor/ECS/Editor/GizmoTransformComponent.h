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

// How the gizmo draws its line geometry in one render pass
struct GizmoDrawStyle
{
	float lineWidth= 1.f;
	float colorScale= 1.f;
	bool bDrawLabels= true;
};

class GizmoTransformComponent : public TransformComponent
{
public:
	GizmoTransformComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName= "GizmoTransformComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	// Handle dimensions, all as fractions of the per-frame display scale
	static constexpr float k_gizmoBaseRadius= 0.5f;
	static constexpr float k_gizmoBaseWidth= 0.05f;
	// Translate arrows: solid shaft and cone head. The axis colliders take the
	// head radius as their half thickness so the hit zone matches the widest
	// visible part of the arrow.
	static constexpr float k_gizmoArrowShaftRadius= 0.03f;
	static constexpr float k_gizmoArrowHeadRadius= 0.07f;
	static constexpr float k_gizmoArrowHeadLengthFraction= 0.25f;
	static constexpr float k_gizmoPlanarHandleFraction= 0.2f;
	static constexpr int k_gizmoArrowSegments= 16;
	static constexpr float k_gizmoScreenSizeFactor= 0.2f;
	// Sized so the gizmo covers the same screen fraction the perspective factor
	// yields at the default camera vfov
	static constexpr float k_gizmoOrthoScreenSizeFactor= 0.65f;
	static constexpr int k_gizmoCircleSegments= 64;
	static constexpr float k_gizmoLineWidth= 4.f;
	static constexpr float k_gizmoOccludedColorScale= 0.35f;

	virtual void init() override;
	virtual void update(float deltaSeconds) override;
	virtual void customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const override;

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
	float m_displayScale= 1.0f;
	bool m_bIsApplyingTransformToTarget= false;
	void renderActiveGizmo(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
						   const GizmoDrawStyle& drawStyle) const;
	float computeDisplayScale() const;
	void updateGizmoColliderScales();
	void applyTransformToTarget();
	void applyTransformToGizmo();

	void onSelectionTranslationRequested(const glm::vec3& worldSpaceTranslation);
	void onSelectionRotationRequested(const glm::quat& worldSpaceRotation);
	void onSelectionScaleRequested(const glm::vec3& objectSpaceScale);

	void onTransformTargetConfigChange(CommonConfigPtr configPtr,
									   const class ConfigPropertyChangeSet& changedPropertySet);
};