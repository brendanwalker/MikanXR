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
{}

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

	SelectionComponentPtr selectionComponentPtr= owner->getComponentOfType<SelectionComponent>();
	selectionComponentPtr->OnInteractionRayOverlapEnter += MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapEnter);
	selectionComponentPtr->OnInteractionRayOverlapExit += MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapExit);
	selectionComponentPtr->OnInteractionGrab += MakeDelegate(this, &GizmoTranslateComponent::onInteractionGrab);
	selectionComponentPtr->OnInteractionMove += MakeDelegate(this, &GizmoTranslateComponent::onInteractionMove);
	selectionComponentPtr->OnInteractionRelease += MakeDelegate(this, &GizmoTranslateComponent::onInteractionRelease);
	m_selectionComponent= selectionComponentPtr;

	m_dragComponent.reset();
	m_dragOrigin= glm::vec3(0.f);
}

void GizmoTranslateComponent::dispose()
{
	MikanComponent::dispose();

	SelectionComponentPtr selectionComponentPtr = m_selectionComponent.lock();
	selectionComponentPtr->OnInteractionRayOverlapEnter -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapEnter);
	selectionComponentPtr->OnInteractionRayOverlapExit -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapExit);
	selectionComponentPtr->OnInteractionGrab -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionGrab);
	selectionComponentPtr->OnInteractionMove -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionMove);
	selectionComponentPtr->OnInteractionRelease -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRelease);
}

glm::vec3 GizmoTranslateComponent::getColliderColor(
	BoxColliderComponentWeakPtr colliderPtr, 
	const glm::vec3& defaultColor) const
{
	if (colliderPtr.lock() == m_dragComponent.lock())
		return Colors::Yellow;
	else if (colliderPtr.lock() == m_dragComponent.lock())
		return Colors::LightGray;
	else
		return Colors::DarkGray;
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

	const glm::vec3 origin = glm_mat4_position(centerCollidePtr->getWorldTransform());
	const glm::vec3 axisCenter = glm_mat4_position(axisCollidePtr->getWorldTransform());
	const glm::vec3 axisEnd= origin + (axisCenter - origin) * 2.f;

	drawArrow(glm::mat4(1.f), origin, axisEnd, 0.05f, color);
}

void GizmoTranslateComponent::renderLines() const
{
	if (m_bEnabled)
	{
		drawTranslationBoxHandle(m_centerHandle, getColliderColor(m_centerHandle, Colors::DarkGray));
		drawTranslationBoxHandle(m_xyHandle, getColliderColor(m_xyHandle, Colors::DarkGray));
		drawTranslationBoxHandle(m_xzHandle, getColliderColor(m_xzHandle, Colors::DarkGray));
		drawTranslationBoxHandle(m_yzHandle, getColliderColor(m_yzHandle, Colors::DarkGray));
		drawTranslationArrowHandle(m_centerHandle, m_xAxisHandle, getColliderColor(m_xAxisHandle, Colors::Red));
		drawTranslationArrowHandle(m_centerHandle, m_yAxisHandle, getColliderColor(m_yAxisHandle, Colors::Green));
		drawTranslationArrowHandle(m_centerHandle, m_zAxisHandle, getColliderColor(m_zAxisHandle, Colors::Blue));
	}
}

void GizmoTranslateComponent::setEnabled(bool bEnabled)
{
	if (m_bEnabled != bEnabled)
	{
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
	m_dragComponent = hitResult.hitComponent;
	m_dragOrigin= hitResult.hitLocation;
}

void GizmoTranslateComponent::onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	ColliderComponentPtr dragColliderPtr= m_dragComponent.lock();
	ColliderComponentPtr centerColliderPtr= m_centerHandle.lock();

	const glm::mat4 centerXform= centerColliderPtr->getWorldTransform();
	const glm::vec3 origin= glm_mat4_position(centerXform);
	const glm::vec3 xAxis= glm_mat4_forward(centerXform);
	const glm::vec3 yAxis = glm_mat4_up(centerXform);
	const glm::vec3 zAxis= glm_mat4_right(centerXform);
	
	float int_time = 0.f;
	glm::vec3 int_point= m_dragOrigin;
	bool has_int= false;

	// Center handle drag
	if (dragColliderPtr == m_centerHandle.lock())
	{
		has_int= glm_closest_point_on_ray_to_point(
			rayOrigin, rayDir, m_dragOrigin,
			int_time, int_point);
	}
	// XY handle drag
	else if (dragColliderPtr == m_xyHandle.lock())
	{		
		has_int= glm_intersect_plane_with_ray(
			origin, zAxis,
			rayOrigin, rayDir, 
			int_time, int_point);
	}
	// XZ handle drag
	else if (dragColliderPtr == m_xzHandle.lock())
	{
		has_int = glm_intersect_plane_with_ray(
			origin, yAxis,
			rayOrigin, rayDir,
			int_time, int_point);
	}
	// YZ handle drag
	else if (dragColliderPtr == m_yzHandle.lock())
	{
		has_int = glm_intersect_plane_with_ray(
			origin, xAxis,
			rayOrigin, rayDir,
			int_time, int_point);
	}
	// X Axis drag
	else if (dragColliderPtr == m_xAxisHandle.lock())
	{
		has_int = glm_closest_point_on_ray_to_ray(
			rayOrigin, rayDir, 
			origin, xAxis, 
			int_time, int_point);
	}
	// Y Axis drag
	else if (dragColliderPtr == m_yAxisHandle.lock())
	{
		has_int = glm_closest_point_on_ray_to_ray(
			rayOrigin, rayDir,
			origin, yAxis,
			int_time, int_point);
	}
	// Z Axis drag
	else if (dragColliderPtr == m_zAxisHandle.lock())
	{
		has_int = glm_closest_point_on_ray_to_ray(
			rayOrigin, rayDir,
			origin, zAxis,
			int_time, int_point);
	}

	if (has_int)
	{
		const glm::vec3 translation = int_point - m_dragOrigin;
		requestTranslation(translation);
	}
}

void GizmoTranslateComponent::onInteractionRelease()
{
	m_dragComponent.reset();
}

void GizmoTranslateComponent::requestTranslation(const glm::vec3& worldSpaceTranslation)
{
	if (OnTranslationRequested)
		OnTranslationRequested(worldSpaceTranslation);
}