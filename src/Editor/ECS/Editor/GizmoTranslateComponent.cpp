#pragma once

#include "BoxColliderComponent.h"
#include "Colors.h"
#include "DiskColliderComponent.h"
#include "EditorObjectSystem.h"
#include "GizmoTransformComponent.h"
#include "GizmoTranslateComponent.h"
#include "MikanCamera.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "SelectionComponent.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "TextStyle.h"

#include <glm/ext/quaternion_float.hpp>

GizmoTranslateComponent::GizmoTranslateComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

void GizmoTranslateComponent::init()
{
	MikanComponent::init();

	MikanObjectPtr owner= getOwnerObject();

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

	MikanComponent::dispose();
}

glm::vec3 GizmoTranslateComponent::getColliderColor(
	BoxColliderComponentWeakPtr colliderPtr,
	const glm::vec3& defaultColor,
	const glm::vec3& hilightColor) const
{
	if (colliderPtr.lock() == m_dragComponent.lock())
		return Colors::Yellow;
	else if (colliderPtr.lock() == m_hoverComponent.lock())
		return hilightColor;
	else
		return defaultColor;
}

static void drawTranslationBoxHandle(BoxColliderComponentWeakPtr colliderWeakPtr, const glm::vec3 color)
{
	BoxColliderComponentPtr collidePtr= colliderWeakPtr.lock();
	IMkGraphicsContext* graphicsContext= collidePtr->getGraphicsContext();

	const glm::mat4 xform= collidePtr->getWorldTransform();
	const glm::vec3 halfExtents= collidePtr->getHalfExtents();
	drawTransformedBox(graphicsContext, xform, halfExtents, color);
}

static void drawTranslationArrowHandle(
	DiskColliderComponentWeakPtr centerColliderWeakPtr,
	BoxColliderComponentWeakPtr axisColliderWeakPtr,
	const glm::vec3 color)
{
	DiskColliderComponentPtr centerCollidePtr= centerColliderWeakPtr.lock();
	BoxColliderComponentPtr axisCollidePtr= axisColliderWeakPtr.lock();
	IMkGraphicsContext* graphicsContext= centerCollidePtr->getGraphicsContext();

	const glm::vec3 origin= glm_mat4_get_position(centerCollidePtr->getWorldTransform());
	const glm::vec3 axisCenter= glm_mat4_get_position(axisCollidePtr->getWorldTransform());
	const glm::vec3 axisEnd= origin + (axisCenter - origin) * 2.f;

	drawArrow(graphicsContext, glm::mat4(1.f), origin, axisEnd, 0.1f, color);
}

void GizmoTranslateComponent::customRender(
	IMkGraphicsContext* graphicsContext,
	MikanCameraPtr viewportCamera) const
{
	if (m_bEnabled)
	{
		drawTranslationBoxHandle(m_xyHandle, getColliderColor(m_xyHandle, Colors::DarkGray, Colors::LightGray));
		drawTranslationBoxHandle(m_xzHandle, getColliderColor(m_xzHandle, Colors::DarkGray, Colors::LightGray));
		drawTranslationBoxHandle(m_yzHandle, getColliderColor(m_yzHandle, Colors::DarkGray, Colors::LightGray));

		drawTranslationArrowHandle(m_viewPlaneHandle, m_xAxisHandle, getColliderColor(m_xAxisHandle, Colors::Red, Colors::Pink));
		drawTranslationArrowHandle(m_viewPlaneHandle, m_yAxisHandle, getColliderColor(m_yAxisHandle, Colors::Green, Colors::LightGreen));
		drawTranslationArrowHandle(m_viewPlaneHandle, m_zAxisHandle, getColliderColor(m_zAxisHandle, Colors::Blue, Colors::LightBlue));

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

			drawTransformedCircle(
				graphicsContext, vph->getWorldTransform(), vph->getRadius(), vphColor,
				GizmoTransformComponent::k_gizmoCircleSegments);
		}
	}
}

void GizmoTranslateComponent::updateColliderScales(float displayScale)
{
	const float R= GizmoTransformComponent::k_gizmoBaseRadius * displayScale;
	const float W= GizmoTransformComponent::k_gizmoBaseWidth * displayScale;
	const float P= R * 0.1f;

	if (auto h= m_xyHandle.lock())
	{
		h->setRelativePosition({P, P, 0});
		h->setHalfExtents({P, P, W * 0.1f});
	}
	if (auto h= m_xzHandle.lock())
	{
		h->setRelativePosition({P, 0, P});
		h->setHalfExtents({P, W * 0.1f, P});
	}
	if (auto h= m_yzHandle.lock())
	{
		h->setRelativePosition({0, P, P});
		h->setHalfExtents({W * 0.1f, P, P});
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
		h->setRadius(W * 2.5f);

		auto editorSystem= getObjectSystemOfType<EditorObjectSystem>();
		if (editorSystem)
		{
			MikanCameraPtr camera= editorSystem->getPrimaryCamera();
			if (camera)
			{
				const glm::vec3 cameraForward= camera->getCameraForwardFromViewMatrix();
				const glm::vec3 yAxis(0.f, 1.f, 0.f);
				const glm::vec3 rotAxis= glm::cross(yAxis, cameraForward);
				const float crossLen= glm::length(rotAxis);
				const float dotVal= glm::clamp(glm::dot(yAxis, cameraForward), -1.f, 1.f);
				const float angle= acosf(dotVal);
				const glm::quat faceCamera=
					(crossLen > 1e-6f)
						? glm::angleAxis(angle, rotAxis / crossLen)
						: glm::quat(1.f, 0.f, 0.f, 0.f);
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
			selectionComponentPtr->OnInteractionRayOverlapEnter+= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit+= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab+= MakeDelegate(this, &GizmoTranslateComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove+= MakeDelegate(this, &GizmoTranslateComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease+= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRelease);
		}
		else
		{
			selectionComponentPtr->OnInteractionRayOverlapEnter-= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit-= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab-= MakeDelegate(this, &GizmoTranslateComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove-= MakeDelegate(this, &GizmoTranslateComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease-= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRelease);
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
			hasClosestPoint= glm_intersect_plane_with_ray(
				m_dragOrigin, m_viewPlaneDragNormal,
				rayOrigin, rayDir,
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
		hasClosestPoint= glm_intersect_plane_with_ray(
			origin, zAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// XZ handle drag
	else if (dragColliderPtr == m_xzHandle.lock())
	{
		hasClosestPoint= glm_intersect_plane_with_ray(
			origin, yAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// YZ handle drag
	else if (dragColliderPtr == m_yzHandle.lock())
	{
		hasClosestPoint= glm_intersect_plane_with_ray(
			origin, xAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// X Axis drag
	else if (dragColliderPtr == m_xAxisHandle.lock())
	{
		hasClosestPoint= glm_closest_point_on_ray_to_ray(
			origin, xAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// Y Axis drag
	else if (dragColliderPtr == m_yAxisHandle.lock())
	{
		hasClosestPoint= glm_closest_point_on_ray_to_ray(
			origin, yAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// Z Axis drag
	else if (dragColliderPtr == m_zAxisHandle.lock())
	{
		hasClosestPoint= glm_closest_point_on_ray_to_ray(
			origin, zAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
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