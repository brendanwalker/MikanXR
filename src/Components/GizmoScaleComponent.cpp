#pragma once

#include "App.h"
#include "BoxColliderComponent.h"
#include "Colors.h"
#include "GizmoScaleComponent.h"
#include "GlLineRenderer.h"
#include "SelectionComponent.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "ProfileConfig.h"

#include "SDL_mouse.h"

static const float k_dragScaleFactor= 1.f;

GizmoScaleComponent::GizmoScaleComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{ }

void GizmoScaleComponent::init()
{
	MikanComponent::init();

	MikanObjectPtr owner = getOwnerObject();

	m_centerHandle = owner->getComponentOfTypeAndName<BoxColliderComponent>("centerScaleHandle");
	m_xAxisHandle = owner->getComponentOfTypeAndName<BoxColliderComponent>("xAxisScaleHandle");
	m_yAxisHandle = owner->getComponentOfTypeAndName<BoxColliderComponent>("yAxisScaleHandle");
	m_zAxisHandle = owner->getComponentOfTypeAndName<BoxColliderComponent>("zAxisScaleHandle");

	m_selectionComponent = owner->getComponentOfType<SelectionComponent>();

	m_dragComponent.reset();
	m_dragOrigin = glm::vec3(0.f);
	m_lastClosestPoint = glm::vec3(0.f);
}

void GizmoScaleComponent::dispose()
{
	setEnabled(false);
	MikanComponent::dispose();
}

glm::vec3 GizmoScaleComponent::getColliderColor(
	BoxColliderComponentWeakPtr colliderPtr,
	const glm::vec3& defaultColor) const
{
	if (colliderPtr.lock() == m_dragComponent.lock())
		return Colors::Yellow;
	else if (colliderPtr.lock() == m_hoverComponent.lock())
		return Colors::LightGray;
	else
		return defaultColor;
}

static void drawScaleBoxHandle(BoxColliderComponentWeakPtr colliderWeakPtr, const glm::vec3 color)
{
	BoxColliderComponentPtr collidePtr = colliderWeakPtr.lock();

	const glm::mat4 xform = collidePtr->getWorldTransform();
	const glm::vec3 halfExtents = collidePtr->getHalfExtents();
	drawTransformedBox(xform, halfExtents, color);
}

static void drawScaleArrowHandle(
	BoxColliderComponentWeakPtr centerColliderWeakPtr,
	BoxColliderComponentWeakPtr axisColliderWeakPtr,
	const glm::vec3 color)
{
	BoxColliderComponentPtr axisCollidePtr = axisColliderWeakPtr.lock();
	const glm::mat4 axisBoxXform = axisCollidePtr->getWorldTransform();
	const glm::vec3 axisBoxHalfExtents = axisCollidePtr->getHalfExtents();
	drawTransformedBox(axisBoxXform, axisBoxHalfExtents, color);

	BoxColliderComponentPtr centerCollidePtr = centerColliderWeakPtr.lock();
	const glm::vec3 origin = glm_mat4_get_position(centerCollidePtr->getWorldTransform());
	const glm::vec3 axisBoxCenter = axisBoxXform * glm::vec4(glm::vec3(0.f), 1.f);
	drawSegment(glm::mat4(1.f), origin, axisBoxCenter, color);
}

void GizmoScaleComponent::customRender()
{
	if (m_bEnabled)
	{
		drawScaleBoxHandle(m_centerHandle, getColliderColor(m_centerHandle, Colors::DarkGray));
		drawScaleArrowHandle(m_centerHandle, m_xAxisHandle, getColliderColor(m_xAxisHandle, Colors::Red));
		drawScaleArrowHandle(m_centerHandle, m_yAxisHandle, getColliderColor(m_yAxisHandle, Colors::Green));
		drawScaleArrowHandle(m_centerHandle, m_zAxisHandle, getColliderColor(m_zAxisHandle, Colors::Blue));
	}
}

void GizmoScaleComponent::setEnabled(bool bEnabled)
{
	if (m_bEnabled != bEnabled)
	{
		SelectionComponentPtr selectionComponentPtr = m_selectionComponent.lock();

		if (bEnabled)
		{
			selectionComponentPtr->OnInteractionRayOverlapEnter += MakeDelegate(this, &GizmoScaleComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit += MakeDelegate(this, &GizmoScaleComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab += MakeDelegate(this, &GizmoScaleComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove += MakeDelegate(this, &GizmoScaleComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease += MakeDelegate(this, &GizmoScaleComponent::onInteractionRelease);
		}
		else
		{
			selectionComponentPtr->OnInteractionRayOverlapEnter -= MakeDelegate(this, &GizmoScaleComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit -= MakeDelegate(this, &GizmoScaleComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab -= MakeDelegate(this, &GizmoScaleComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove -= MakeDelegate(this, &GizmoScaleComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease -= MakeDelegate(this, &GizmoScaleComponent::onInteractionRelease);
		}

		m_centerHandle.lock()->setEnabled(bEnabled);
		m_xAxisHandle.lock()->setEnabled(bEnabled);
		m_yAxisHandle.lock()->setEnabled(bEnabled);
		m_zAxisHandle.lock()->setEnabled(bEnabled);
		m_bEnabled= bEnabled;
	}
}

void GizmoScaleComponent::onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult)
{
	m_hoverComponent = hitResult.hitComponent;
}

void GizmoScaleComponent::onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult)
{
	m_hoverComponent.reset();
}

void GizmoScaleComponent::onInteractionGrab(const ColliderRaycastHitResult& hitResult)
{
	m_dragComponent = hitResult.hitComponent;
	m_dragOrigin = hitResult.hitLocation;
	m_lastClosestPoint= m_dragOrigin;
}

static float computeScaleDelta(
	const glm::vec3& gizmo_center,
	const glm::vec3& drag_origin,
	const glm::vec3& drag_start,
	const glm::vec3& drag_end)
{
	const float dragDistance= glm::length(drag_start - drag_end);
	const float deltaMagnitude = dragDistance * k_dragScaleFactor;
	const float dragSign= sgn(glm::dot(drag_end - drag_start, drag_start - gizmo_center));
	const float originSign= sgn(glm::dot(drag_origin - gizmo_center, drag_start - gizmo_center));

	return originSign * dragSign * deltaMagnitude;
}

void GizmoScaleComponent::onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	ColliderComponentPtr dragColliderPtr = m_dragComponent.lock();
	ColliderComponentPtr centerColliderPtr = m_centerHandle.lock();

	const glm::mat4 centerXform = centerColliderPtr->getWorldTransform();
	const glm::vec3 origin = glm_mat4_get_position(centerXform);
	const glm::vec3 xAxis = glm_mat4_get_x_axis(centerXform);
	const glm::vec3 yAxis = glm_mat4_get_y_axis(centerXform);
	const glm::vec3 zAxis = glm_mat4_get_z_axis(centerXform);

	float closestTime = 0.f;
	glm::vec3 closestPoint = m_dragOrigin;
	bool hasClosestPoint = false;

	glm::vec3 scaleDelta(0.f, 0.f, 0.f);

	// Center handle drag
	if (dragColliderPtr == m_centerHandle.lock())
	{
		hasClosestPoint = glm_closest_point_on_ray_to_point(
			rayOrigin, rayDir, m_dragOrigin,
			closestTime, closestPoint);
		
		const float delta= computeScaleDelta(origin, m_dragOrigin, m_lastClosestPoint, closestPoint);
		scaleDelta= glm::vec3(delta, delta, delta);
	}
	// X Axis drag
	else if (dragColliderPtr == m_xAxisHandle.lock())
	{
		hasClosestPoint = glm_closest_point_on_ray_to_ray(
			rayOrigin, rayDir,
			origin, xAxis,
			closestTime, closestPoint);

		const float delta = computeScaleDelta(origin, m_dragOrigin, m_lastClosestPoint, closestPoint);
		scaleDelta = glm::vec3(delta, 0.f, 0.f);
	}
	// Y Axis drag
	else if (dragColliderPtr == m_yAxisHandle.lock())
	{
		hasClosestPoint = glm_closest_point_on_ray_to_ray(
			rayOrigin, rayDir,
			origin, yAxis,
			closestTime, closestPoint);

		const float delta = computeScaleDelta(origin, m_dragOrigin, m_lastClosestPoint, closestPoint);
		scaleDelta = glm::vec3(0.f, delta, 0.f);
	}
	// Z Axis drag
	else if (dragColliderPtr == m_zAxisHandle.lock())
	{
		hasClosestPoint = glm_closest_point_on_ray_to_ray(
			rayOrigin, rayDir,
			origin, zAxis,
			closestTime, closestPoint);

		const float delta = computeScaleDelta(origin, m_dragOrigin, m_lastClosestPoint, closestPoint);
		scaleDelta = glm::vec3(0.f, 0.f, delta);
	}

	if (hasClosestPoint)
	{
		requestScale(scaleDelta);
		m_lastClosestPoint = closestPoint;
	}
}

void GizmoScaleComponent::onInteractionRelease()
{
	m_dragComponent.reset();
}

void GizmoScaleComponent::requestScale(const glm::vec3& objectSpaceScale)
{
	if (OnScaleRequested)
		OnScaleRequested(objectSpaceScale);
}