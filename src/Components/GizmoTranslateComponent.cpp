#pragma once

#include "BoxColliderComponent.h"
#include "Colors.h"
#include "GizmoTranslateComponent.h"
#include "GlLineRenderer.h"
#include "SelectionComponent.h"
#include "MathGLM.h"
#include "MikanObject.h"

#include "SDL_mouse.h"

GizmoTranslateComponent::GizmoTranslateComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{ }

void GizmoTranslateComponent::init()
{
	MikanComponent::init();

	MikanObjectPtr owner = getOwnerObject();

	m_centerHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("centerTranslateHandle");
	m_xyHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("xyTranslateHandle");
	m_xzHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("xzTranslateHandle");
	m_yzHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("yzTranslateHandle");
	m_xAxisHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("xAxisTranslateHandle");
	m_yAxisHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("yAxisTranslateHandle");
	m_zAxisHandle= owner->getComponentOfTypeAndName<BoxColliderComponent>("zAxisTranslateHandle");

	m_selectionComponent= owner->getComponentOfType<SelectionComponent>();

	m_dragComponent.reset();
	m_dragOrigin= glm::vec3(0.f);
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
	BoxColliderComponentPtr collidePtr = colliderWeakPtr.lock();

	const glm::mat4 xform = collidePtr->getWorldTransform();
	const glm::vec3 halfExtents = collidePtr->getHalfExtents();
	drawTransformedBox(xform, halfExtents, color);
}

static void drawTranslationArrowHandle(
	BoxColliderComponentWeakPtr centerColliderWeakPtr,
	BoxColliderComponentWeakPtr axisColliderWeakPtr,
	const glm::vec3 color)
{
	BoxColliderComponentPtr centerCollidePtr = centerColliderWeakPtr.lock();
	BoxColliderComponentPtr axisCollidePtr = axisColliderWeakPtr.lock();

	const glm::vec3 origin = glm_mat4_get_position(centerCollidePtr->getWorldTransform());
	const glm::vec3 axisCenter = glm_mat4_get_position(axisCollidePtr->getWorldTransform());
	const glm::vec3 axisEnd= origin + (axisCenter - origin) * 2.f;

	drawArrow(glm::mat4(1.f), origin, axisEnd, 0.05f, color);
}

void GizmoTranslateComponent::customRender()
{
	if (m_bEnabled)
	{
		drawTranslationBoxHandle(m_centerHandle, getColliderColor(m_centerHandle, Colors::DarkGray, Colors::LightGray));
		drawTranslationBoxHandle(m_xyHandle, getColliderColor(m_xyHandle, Colors::DarkGray, Colors::LightGray));
		drawTranslationBoxHandle(m_xzHandle, getColliderColor(m_xzHandle, Colors::DarkGray, Colors::LightGray));
		drawTranslationBoxHandle(m_yzHandle, getColliderColor(m_yzHandle, Colors::DarkGray, Colors::LightGray));

		drawTranslationArrowHandle(m_centerHandle, m_xAxisHandle, getColliderColor(m_xAxisHandle, Colors::Red,  Colors::Pink));
		drawTranslationBoxHandle(m_xAxisHandle, getColliderColor(m_xAxisHandle, Colors::DarkGray, Colors::LightGray));

		drawTranslationArrowHandle(m_centerHandle, m_yAxisHandle, getColliderColor(m_yAxisHandle, Colors::Green, Colors::LightGreen));
		drawTranslationBoxHandle(m_yAxisHandle, getColliderColor(m_yAxisHandle, Colors::DarkGray, Colors::LightGray));

		drawTranslationArrowHandle(m_centerHandle, m_zAxisHandle, getColliderColor(m_zAxisHandle, Colors::Blue, Colors::LightBlue));
		drawTranslationBoxHandle(m_zAxisHandle, getColliderColor(m_zAxisHandle, Colors::DarkGray, Colors::LightGray));
	}
}

void GizmoTranslateComponent::setEnabled(bool bEnabled)
{
	if (m_bEnabled != bEnabled)
	{
		SelectionComponentPtr selectionComponentPtr = m_selectionComponent.lock();

		if (bEnabled)
		{
			selectionComponentPtr->OnInteractionRayOverlapEnter += MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit += MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab += MakeDelegate(this, &GizmoTranslateComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove += MakeDelegate(this, &GizmoTranslateComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease += MakeDelegate(this, &GizmoTranslateComponent::onInteractionRelease);
		}
		else
		{
			selectionComponentPtr->OnInteractionRayOverlapEnter -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRelease);
		}

		m_centerHandle.lock()->setEnabled(bEnabled);
		m_xyHandle.lock()->setEnabled(bEnabled);
		m_xzHandle.lock()->setEnabled(bEnabled);
		m_yzHandle.lock()->setEnabled(bEnabled);
		m_xAxisHandle.lock()->setEnabled(bEnabled);
		m_yAxisHandle.lock()->setEnabled(bEnabled);
		m_zAxisHandle.lock()->setEnabled(bEnabled);
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
	ColliderComponentPtr dragColliderPtr = hitResult.hitComponent.lock();

	m_dragComponent = hitResult.hitComponent;
}

void GizmoTranslateComponent::onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	ColliderComponentPtr dragColliderPtr = m_dragComponent.lock();
	ColliderComponentPtr centerColliderPtr = m_centerHandle.lock();

	const glm::mat4 centerXform = centerColliderPtr->getWorldTransform();
	const glm::vec3 origin = glm_mat4_get_position(centerXform);
	const glm::vec3 xAxis = glm::normalize(glm_mat4_get_x_axis(centerXform));
	const glm::vec3 yAxis = glm::normalize(glm_mat4_get_y_axis(centerXform));
	const glm::vec3 zAxis = glm::normalize(glm_mat4_get_z_axis(centerXform));

	float closestTime = 0.f;
	glm::vec3 closestPoint = rayOrigin;
	bool hasClosestPoint = false;

	// Center handle drag
	if (dragColliderPtr == m_centerHandle.lock())
	{
		if (m_bValidDragOrigin)
		{
			hasClosestPoint = glm_closest_point_on_ray_to_point(
				rayOrigin, rayDir, m_dragOrigin,
				closestTime, closestPoint);
		}
		else
		{
			closestPoint = dragColliderPtr->getWorldLocation();
			hasClosestPoint = true;
		}
	}
	// XY handle drag
	else if (dragColliderPtr == m_xyHandle.lock())
	{
		hasClosestPoint = glm_intersect_plane_with_ray(
			origin, zAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// XZ handle drag
	else if (dragColliderPtr == m_xzHandle.lock())
	{
		hasClosestPoint = glm_intersect_plane_with_ray(
			origin, yAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// YZ handle drag
	else if (dragColliderPtr == m_yzHandle.lock())
	{
		hasClosestPoint = glm_intersect_plane_with_ray(
			origin, xAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// X Axis drag
	else if (dragColliderPtr == m_xAxisHandle.lock())
	{
		hasClosestPoint = glm_closest_point_on_ray_to_ray(
			origin, xAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// Y Axis drag
	else if (dragColliderPtr == m_yAxisHandle.lock())
	{
		hasClosestPoint = glm_closest_point_on_ray_to_ray(
			origin, yAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}
	// Z Axis drag
	else if (dragColliderPtr == m_zAxisHandle.lock())
	{
		hasClosestPoint = glm_closest_point_on_ray_to_ray(
			origin, zAxis,
			rayOrigin, rayDir,
			closestTime, closestPoint);
	}

	if (hasClosestPoint)
	{
		if (m_bValidDragOrigin)
		{
			// Compute the world space drag delta
			const glm::vec3 worldSpaceTranslation = closestPoint - m_dragOrigin;
			m_dragOrigin = closestPoint;

			requestTranslation(worldSpaceTranslation);
		}
		else
		{
			m_dragOrigin = closestPoint;
			m_bValidDragOrigin = true;
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