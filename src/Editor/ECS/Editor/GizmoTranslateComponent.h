#pragma once

#include "ColliderQuery.h"
#include "ObjectSystemFwd.h"
#include "MikanComponent.h"
#include "MkRendererFwd.h"
#include "SinglecastDelegate.h"

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class GizmoTranslateComponent : public MikanComponent
{
public:
	GizmoTranslateComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName= "GizmoTranslateComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	virtual void init() override;
	virtual void customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const override;
	void customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
					  const struct GizmoDrawStyle& drawStyle) const;
	virtual void dispose() override;

	void setEnabled(bool bEnabled);
	void updateColliderScales(float displayScale);

	SinglecastDelegate<void(const glm::vec3& position)> OnTranslationRequested;

protected:
	glm::vec3 getColliderColor(BoxColliderComponentWeakPtr colliderPtr, const glm::vec3& defaultColor,
							   const glm::vec3& hilightColor) const;

	// Solid arrow geometry: a unit cylinder and a unit cone along +Z, scaled
	// and rotated onto each axis at draw time
	void createArrowMeshes();
	void drawArrowHandle(MikanCameraPtr viewportCamera, const glm::mat4& axisXform, const glm::vec3& color,
						 const struct GizmoDrawStyle& drawStyle) const;

	void onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult);
	void onInteractionGrab(const ColliderRaycastHitResult& hitResult);
	void onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onInteractionRelease();

	void requestTranslation(const glm::vec3& worldSpaceTranslation);

	bool m_bEnabled= false;
	BoxColliderComponentWeakPtr m_xyHandle;
	BoxColliderComponentWeakPtr m_xzHandle;
	BoxColliderComponentWeakPtr m_yzHandle;
	BoxColliderComponentWeakPtr m_xAxisHandle;
	BoxColliderComponentWeakPtr m_yAxisHandle;
	BoxColliderComponentWeakPtr m_zAxisHandle;
	DiskColliderComponentWeakPtr m_viewPlaneHandle;

	SelectionComponentWeakPtr m_selectionComponent;
	ColliderComponentWeakPtr m_hoverComponent;
	ColliderComponentWeakPtr m_dragComponent;
	glm::vec3 m_dragOrigin;
	glm::vec3 m_viewPlaneDragNormal;
	bool m_bValidDragOrigin= false;

	IMkTriangulatedMeshPtr m_arrowShaftMesh;
	IMkTriangulatedMeshPtr m_arrowHeadMesh;
	float m_displayScale= 1.f;
};