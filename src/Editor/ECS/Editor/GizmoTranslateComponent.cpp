#pragma once

#include "BoxColliderComponent.h"
#include "Colors.h"
#include "DiskColliderComponent.h"
#include "EditorObjectSystem.h"
#include "GizmoTransformComponent.h"
#include "GizmoTranslateComponent.h"
#include "IMkGraphicsContext.h"
#include "IMkTriangulatedMesh.h"
#include "MikanCamera.h"
#include "MikanLineRenderer.h"
#include "MikanShaderCache.h"
#include "MikanTextRenderer.h"
#include "MkMaterialInstance.h"
#include "SelectionComponent.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "TextStyle.h"

#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Rotations taking the unit meshes' +Z onto each gizmo axis
static const glm::mat4 k_xAxisFromZ= glm::rotate(glm::mat4(1.f), glm::half_pi<float>(), glm::vec3(0.f, 1.f, 0.f));
static const glm::mat4 k_yAxisFromZ= glm::rotate(glm::mat4(1.f), -glm::half_pi<float>(), glm::vec3(1.f, 0.f, 0.f));
static const glm::mat4 k_zAxisFromZ= glm::mat4(1.f);

GizmoTranslateComponent::GizmoTranslateComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

void GizmoTranslateComponent::init()
{
	MikanComponent::init();

	MikanObjectPtr owner= getOwnerObject();

	createArrowMeshes();

	m_xyHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("xyTranslateHandle");
	m_xzHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("xzTranslateHandle");
	m_yzHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("yzTranslateHandle");
	m_xAxisHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("xAxisTranslateHandle");
	m_yAxisHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("yAxisTranslateHandle");
	m_zAxisHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("zAxisTranslateHandle");
	m_viewPlaneHandle= owner->getComponentOfTypeAndName<DiskColliderComponent>("viewPlaneTranslateHandle");

	m_selectionComponent= owner->getComponentOfType<SelectionComponent>();

	m_dragComponent.reset();
	m_dragOrigin= glm::vec3(0.f);
	m_viewPlaneDragNormal= glm::vec3(0.f, 0.f, 1.f);
}

void GizmoTranslateComponent::dispose()
{
	setEnabled(false);

	m_arrowShaftMesh= nullptr;
	m_arrowHeadMesh= nullptr;

	MikanComponent::dispose();
}

void GizmoTranslateComponent::createArrowMeshes()
{
	// Headless runs have no window or graphics context, and draw nothing
	IMkGraphicsContext* graphicsContext= getGraphicsContext();
	if (!graphicsContext)
		return;

	constexpr int N= GizmoTransformComponent::k_gizmoArrowSegments;
	struct PosVert
	{
		float x, y, z;
	};

	// Shaft: unit cylinder from z=0 to z=1, radius 1, capped at z=0. The
	// z=1 end sits inside the wider cone base, so it needs no cap.
	{
		// Rings interleaved: base vertex i at 2i, top vertex i at 2i+1
		std::vector<PosVert> verts;
		verts.reserve(2 * N + 1);
		for (int i= 0; i < N; ++i)
		{
			const float a= (float)i / (float)N * (2.f * glm::pi<float>());
			verts.push_back({cosf(a), sinf(a), 0.f});
			verts.push_back({cosf(a), sinf(a), 1.f});
		}
		verts.push_back({0.f, 0.f, 0.f}); // base center: 2N

		std::vector<uint32_t> indices;
		indices.reserve(3 * N * 3);
		for (int i= 0; i < N; ++i)
		{
			const uint32_t b0= 2 * i, t0= 2 * i + 1;
			const uint32_t b1= 2 * ((i + 1) % N), t1= 2 * ((i + 1) % N) + 1;
			// Side quad as two triangles
			indices.insert(indices.end(), {b0, b1, t1});
			indices.insert(indices.end(), {b0, t1, t0});
			// Base cap fan
			indices.insert(indices.end(), {(uint32_t)(2 * N), b1, b0});
		}

		m_arrowShaftMesh= createMkTriangulatedMesh(
			graphicsContext, "gizmoArrowShaft", reinterpret_cast<const uint8_t*>(verts.data()), sizeof(PosVert),
			(uint32_t)verts.size(), reinterpret_cast<const uint8_t*>(indices.data()), sizeof(uint32_t),
			(uint32_t)(indices.size() / 3), false);
	}

	// Head: unit cone with its base ring at z=0 and tip at z=1, radius 1
	{
		std::vector<PosVert> verts;
		verts.reserve(N + 2);
		for (int i= 0; i < N; ++i)
		{
			const float a= (float)i / (float)N * (2.f * glm::pi<float>());
			verts.push_back({cosf(a), sinf(a), 0.f}); // base ring: 0..N-1
		}
		verts.push_back({0.f, 0.f, 1.f}); // tip: N
		verts.push_back({0.f, 0.f, 0.f}); // base center: N+1

		std::vector<uint32_t> indices;
		indices.reserve(2 * N * 3);
		for (int i= 0; i < N; ++i)
		{
			const uint32_t r0= i, r1= (i + 1) % N;
			indices.insert(indices.end(), {r0, r1, (uint32_t)N});
			indices.insert(indices.end(), {(uint32_t)(N + 1), r1, r0});
		}

		m_arrowHeadMesh= createMkTriangulatedMesh(
			graphicsContext, "gizmoArrowHead", reinterpret_cast<const uint8_t*>(verts.data()), sizeof(PosVert),
			(uint32_t)verts.size(), reinterpret_cast<const uint8_t*>(indices.data()), sizeof(uint32_t),
			(uint32_t)(indices.size() / 3), false);
	}

	MkMaterialConstPtr material= graphicsContext->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_P_SOLID_COLOR);
	for (IMkTriangulatedMeshPtr mesh : {m_arrowShaftMesh, m_arrowHeadMesh})
	{
		if (mesh)
		{
			mesh->setMaterial(material);
			mesh->createResources();
		}
	}
}

void GizmoTranslateComponent::drawArrowHandle(MikanCameraPtr viewportCamera, const glm::mat4& axisXform,
											  const glm::vec3& color, const GizmoDrawStyle& drawStyle) const
{
	if (!m_arrowShaftMesh || !m_arrowHeadMesh)
		return;

	const float R= GizmoTransformComponent::k_gizmoBaseRadius * m_displayScale;
	const float shaftRadius= GizmoTransformComponent::k_gizmoArrowShaftRadius * m_displayScale;
	const float headRadius= GizmoTransformComponent::k_gizmoArrowHeadRadius * m_displayScale;
	const float headLength= R * GizmoTransformComponent::k_gizmoArrowHeadLengthFraction;
	const float shaftLength= R - headLength;

	const glm::vec4 rgba(color * drawStyle.colorScale, 1.f);
	m_arrowShaftMesh->getMaterialInstance()->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, rgba);
	m_arrowHeadMesh->getMaterialInstance()->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, rgba);

	const glm::mat4 shaftXform= glm::scale(axisXform, glm::vec3(shaftRadius, shaftRadius, shaftLength));
	const glm::mat4 headXform= glm::scale(glm::translate(axisXform, glm::vec3(0.f, 0.f, shaftLength)),
										  glm::vec3(headRadius, headRadius, headLength));

	drawTransformedTriangulatedMesh(viewportCamera, shaftXform, m_arrowShaftMesh);
	drawTransformedTriangulatedMesh(viewportCamera, headXform, m_arrowHeadMesh);
}

glm::vec3 GizmoTranslateComponent::getColliderColor(BoxColliderComponentWeakPtr colliderPtr,
													const glm::vec3& defaultColor, const glm::vec3& hilightColor) const
{
	if (colliderPtr.lock() == m_dragComponent.lock())
		return Colors::Yellow;
	else if (colliderPtr.lock() == m_hoverComponent.lock())
		return hilightColor;
	else
		return defaultColor;
}

static void drawTranslationBoxHandle(BoxColliderComponentWeakPtr colliderWeakPtr, const glm::vec3 color,
									 const GizmoDrawStyle& drawStyle)
{
	BoxColliderComponentPtr collidePtr= colliderWeakPtr.lock();
	IMkGraphicsContext* graphicsContext= collidePtr->getGraphicsContext();

	const glm::mat4 xform= collidePtr->getWorldTransform();
	const glm::vec3 halfExtents= collidePtr->getHalfExtents();
	drawTransformedBox(graphicsContext, xform, halfExtents, color * drawStyle.colorScale, drawStyle.lineWidth);
}

void GizmoTranslateComponent::customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const
{
	customRender(graphicsContext, viewportCamera, GizmoDrawStyle());
}

void GizmoTranslateComponent::customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
										   const GizmoDrawStyle& drawStyle) const
{
	if (m_bEnabled)
	{
		drawTranslationBoxHandle(m_xyHandle, getColliderColor(m_xyHandle, Colors::DarkGray, Colors::LightGray),
								 drawStyle);
		drawTranslationBoxHandle(m_xzHandle, getColliderColor(m_xzHandle, Colors::DarkGray, Colors::LightGray),
								 drawStyle);
		drawTranslationBoxHandle(m_yzHandle, getColliderColor(m_yzHandle, Colors::DarkGray, Colors::LightGray),
								 drawStyle);

		// Solid arrows in gizmo space: the root carries the target's rotation and
		// translation with the scale factored out
		const glm::mat4 gizmoXform= getOwnerObject()->getRootComponent()->getWorldTransform();
		drawArrowHandle(viewportCamera, gizmoXform * k_xAxisFromZ,
						getColliderColor(m_xAxisHandle, Colors::Red, Colors::Pink), drawStyle);
		drawArrowHandle(viewportCamera, gizmoXform * k_yAxisFromZ,
						getColliderColor(m_yAxisHandle, Colors::Green, Colors::LightGreen), drawStyle);
		drawArrowHandle(viewportCamera, gizmoXform * k_zAxisFromZ,
						getColliderColor(m_zAxisHandle, Colors::Blue, Colors::LightBlue), drawStyle);

		// View-plane handle: camera-facing circle at center
		if (auto vph= m_viewPlaneHandle.lock())
		{
			TextStyle style= getDefaultTextStyle();

			const glm::vec3 origin= glm_mat4_get_position(vph->getWorldTransform());

			ColliderComponentPtr vphBase= vph;
			glm::vec3 vphColor= Colors::DarkGray;
			if (vphBase == m_dragComponent.lock())
				vphColor= Colors::Yellow;
			else if (vphBase == m_hoverComponent.lock())
				vphColor= Colors::LightGray;

			if (drawStyle.bDrawLabels)
			{
				// Axis labels at arrow tips
				auto drawAxisLabel= [&](BoxColliderComponentWeakPtr axisHandle, const wchar_t* label)
				{
					if (auto axisPtr= axisHandle.lock())
					{
						const glm::vec3 axisCenter= glm_mat4_get_position(axisPtr->getWorldTransform());
						const glm::vec3 tip= origin + (axisCenter - origin) * 2.f;
						drawTextAtWorldPosition(graphicsContext, style, tip, label);
					}
				};

				drawAxisLabel(m_xAxisHandle, L"X");
				drawAxisLabel(m_yAxisHandle, L"Y");
				drawAxisLabel(m_zAxisHandle, L"Z");
			}

			drawTransformedCircle(graphicsContext, vph->getWorldTransform(), vph->getRadius(),
								  vphColor * drawStyle.colorScale, GizmoTransformComponent::k_gizmoCircleSegments,
								  drawStyle.lineWidth);
		}
	}
}

void GizmoTranslateComponent::updateColliderScales(float displayScale)
{
	m_displayScale= displayScale;

	const float R= GizmoTransformComponent::k_gizmoBaseRadius * displayScale;
	// Axis hit zones are as thick as the arrow head, the widest visible part
	const float W= GizmoTransformComponent::k_gizmoArrowHeadRadius * displayScale;
	const float T= GizmoTransformComponent::k_gizmoBaseWidth * displayScale;
	// Planar squares sit out between the shafts, clear of the view-plane ring
	const float P= R * GizmoTransformComponent::k_gizmoPlanarHandleFraction;
	const float PC= P * 1.5f;

	if (auto h= m_xyHandle.lock())
	{
		h->setRelativePosition({PC, PC, 0});
		h->setHalfExtents({P, P, T * 0.1f});
	}
	if (auto h= m_xzHandle.lock())
	{
		h->setRelativePosition({PC, 0, PC});
		h->setHalfExtents({P, T * 0.1f, P});
	}
	if (auto h= m_yzHandle.lock())
	{
		h->setRelativePosition({0, PC, PC});
		h->setHalfExtents({T * 0.1f, P, P});
	}
	if (auto h= m_xAxisHandle.lock())
	{
		h->setRelativePosition({R / 2.f, 0, 0});
		h->setHalfExtents({R / 2.f, W, W});
	}
	if (auto h= m_yAxisHandle.lock())
	{
		h->setRelativePosition({0, R / 2.f, 0});
		h->setHalfExtents({W, R / 2.f, W});
	}
	if (auto h= m_zAxisHandle.lock())
	{
		h->setRelativePosition({0, 0, R / 2.f});
		h->setHalfExtents({W, W, R / 2.f});
	}

	if (auto h= m_viewPlaneHandle.lock())
	{
		h->setRelativePosition({0, 0, 0});
		h->setRadius(T * 2.5f);

		auto editorSystem= getObjectSystemOfType<EditorObjectSystem>();
		if (editorSystem)
		{
			MikanCameraPtr camera= editorSystem->getPrimaryCamera();
			if (camera)
			{
				// The disk's rotation is relative to the gizmo root, which carries the
				// target's rotation, so the camera forward has to be brought into that
				// frame first or a rotated target ends up with an edge-on disk
				const glm::mat3 rootRotation(getOwnerObject()->getRootComponent()->getWorldTransform());
				const glm::vec3 cameraForward=
					glm::normalize(glm::transpose(rootRotation) * camera->getCameraForwardFromViewMatrix());
				const glm::vec3 yAxis(0.f, 1.f, 0.f);
				const glm::vec3 rotAxis= glm::cross(yAxis, cameraForward);
				const float crossLen= glm::length(rotAxis);
				const float dotVal= glm::clamp(glm::dot(yAxis, cameraForward), -1.f, 1.f);
				const float angle= acosf(dotVal);
				const glm::quat faceCamera=
					(crossLen > 1e-6f) ? glm::angleAxis(angle, rotAxis / crossLen) : glm::quat(1.f, 0.f, 0.f, 0.f);
				h->setRelativeRotation(faceCamera);
			}
		}
	}
}

void GizmoTranslateComponent::setEnabled(bool bEnabled)
{
	if (m_bEnabled != bEnabled)
	{
		SelectionComponentPtr selectionComponentPtr= m_selectionComponent.lock();

		if (bEnabled)
		{
			selectionComponentPtr->OnInteractionRayOverlapEnter+=
				MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit+=
				MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab+= MakeDelegate(this, &GizmoTranslateComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove+= MakeDelegate(this, &GizmoTranslateComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease+=
				MakeDelegate(this, &GizmoTranslateComponent::onInteractionRelease);
		}
		else
		{
			selectionComponentPtr->OnInteractionRayOverlapEnter-=
				MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit-=
				MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab-= MakeDelegate(this, &GizmoTranslateComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove-= MakeDelegate(this, &GizmoTranslateComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease-=
				MakeDelegate(this, &GizmoTranslateComponent::onInteractionRelease);
		}

		m_xyHandle.lock()->setEnabled(bEnabled);
		m_xzHandle.lock()->setEnabled(bEnabled);
		m_yzHandle.lock()->setEnabled(bEnabled);
		m_xAxisHandle.lock()->setEnabled(bEnabled);
		m_yAxisHandle.lock()->setEnabled(bEnabled);
		m_zAxisHandle.lock()->setEnabled(bEnabled);
		if (auto h= m_viewPlaneHandle.lock())
			h->setEnabled(bEnabled);
		m_bEnabled= bEnabled;
	}
}

void GizmoTranslateComponent::onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult)
{
	m_hoverComponent= hitResult.hitComponent;
}

void GizmoTranslateComponent::onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult)
{
	m_hoverComponent.reset();
}

void GizmoTranslateComponent::onInteractionGrab(const ColliderRaycastHitResult& hitResult)
{
	m_dragComponent= hitResult.hitComponent;

	DiskColliderComponentPtr vph= m_viewPlaneHandle.lock();
	if (hitResult.hitComponent.lock() == vph)
	{
		auto editorSystem= getObjectSystemOfType<EditorObjectSystem>();
		if (editorSystem)
		{
			MikanCameraPtr camera= editorSystem->getPrimaryCamera();
			if (camera)
				m_viewPlaneDragNormal= camera->getCameraForwardFromViewMatrix();
		}
		m_dragOrigin= hitResult.hitLocation;
		m_bValidDragOrigin= true;
	}
}

void GizmoTranslateComponent::onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	ColliderComponentPtr dragColliderPtr= m_dragComponent.lock();

	// Use the gizmo root's world transform for axes — the view-plane disk has a
	// camera-facing orientation so its axes are screen-space, not gizmo-space.
	const glm::mat4 gizmoXform= getOwnerObject()->getRootComponent()->getWorldTransform();
	const glm::vec3 origin= glm_mat4_get_position(gizmoXform);
	const glm::vec3 xAxis= glm::normalize(glm_mat4_get_x_axis(gizmoXform));
	const glm::vec3 yAxis= glm::normalize(glm_mat4_get_y_axis(gizmoXform));
	const glm::vec3 zAxis= glm::normalize(glm_mat4_get_z_axis(gizmoXform));

	float closestTime= 0.f;
	glm::vec3 closestPoint= rayOrigin;
	bool hasClosestPoint= false;

	// View-plane handle drag (camera-facing plane)
	if (dragColliderPtr == m_viewPlaneHandle.lock())
	{
		if (m_bValidDragOrigin)
		{
			hasClosestPoint= glm_intersect_plane_with_ray(m_dragOrigin, m_viewPlaneDragNormal, rayOrigin, rayDir,
														  closestTime, closestPoint);
		}
		else
		{
			closestPoint= dragColliderPtr->getWorldLocation();
			hasClosestPoint= true;
		}
	}
	// XY handle drag
	else if (dragColliderPtr == m_xyHandle.lock())
	{
		hasClosestPoint= glm_intersect_plane_with_ray(origin, zAxis, rayOrigin, rayDir, closestTime, closestPoint);
	}
	// XZ handle drag
	else if (dragColliderPtr == m_xzHandle.lock())
	{
		hasClosestPoint= glm_intersect_plane_with_ray(origin, yAxis, rayOrigin, rayDir, closestTime, closestPoint);
	}
	// YZ handle drag
	else if (dragColliderPtr == m_yzHandle.lock())
	{
		hasClosestPoint= glm_intersect_plane_with_ray(origin, xAxis, rayOrigin, rayDir, closestTime, closestPoint);
	}
	// X Axis drag
	else if (dragColliderPtr == m_xAxisHandle.lock())
	{
		hasClosestPoint= glm_closest_point_on_ray_to_ray(origin, xAxis, rayOrigin, rayDir, closestTime, closestPoint);
	}
	// Y Axis drag
	else if (dragColliderPtr == m_yAxisHandle.lock())
	{
		hasClosestPoint= glm_closest_point_on_ray_to_ray(origin, yAxis, rayOrigin, rayDir, closestTime, closestPoint);
	}
	// Z Axis drag
	else if (dragColliderPtr == m_zAxisHandle.lock())
	{
		hasClosestPoint= glm_closest_point_on_ray_to_ray(origin, zAxis, rayOrigin, rayDir, closestTime, closestPoint);
	}

	if (hasClosestPoint)
	{
		if (m_bValidDragOrigin)
		{
			// Compute the world space drag delta
			const glm::vec3 worldSpaceTranslation= closestPoint - m_dragOrigin;
			m_dragOrigin= closestPoint;

			requestTranslation(worldSpaceTranslation);
		}
		else
		{
			m_dragOrigin= closestPoint;
			m_bValidDragOrigin= true;
		}
	}
}

void GizmoTranslateComponent::onInteractionRelease()
{
	m_dragComponent.reset();
	m_dragOrigin= glm::vec3(0.f);
	m_bValidDragOrigin= false;
}

void GizmoTranslateComponent::requestTranslation(const glm::vec3& worldSpaceTranslation)
{
	if (OnTranslationRequested)
		OnTranslationRequested(worldSpaceTranslation);
}