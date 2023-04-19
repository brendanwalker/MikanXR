#pragma once

#include "Colors.h"
#include "DiskColliderComponent.h"
#include "GlLineRenderer.h"
#include "GizmoRotateComponent.h"
#include "MikanObject.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "SelectionComponent.h"

#include "SDL_mouse.h"

GizmoRotateComponent::GizmoRotateComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{}

void GizmoRotateComponent::init()
{
	MikanComponent::init();

	MikanObjectPtr owner = getOwnerObject();

	m_xAxisHandle = owner->getComponentOfTypeAndName<DiskColliderComponent>("xAxisRotateHandle");
	m_yAxisHandle = owner->getComponentOfTypeAndName<DiskColliderComponent>("yAxisRotateHandle");
	m_zAxisHandle = owner->getComponentOfTypeAndName<DiskColliderComponent>("zAxisRotateHandle");

	SelectionComponentPtr selectionComponentPtr = owner->getComponentOfType<SelectionComponent>();
	selectionComponentPtr->OnInteractionRayOverlapEnter += MakeDelegate(this, &GizmoRotateComponent::onInteractionRayOverlapEnter);
	selectionComponentPtr->OnInteractionRayOverlapExit += MakeDelegate(this, &GizmoRotateComponent::onInteractionRayOverlapExit);
	selectionComponentPtr->OnInteractionGrab += MakeDelegate(this, &GizmoRotateComponent::onInteractionGrab);
	selectionComponentPtr->OnInteractionMove += MakeDelegate(this, &GizmoRotateComponent::onInteractionMove);
	selectionComponentPtr->OnInteractionRelease += MakeDelegate(this, &GizmoRotateComponent::onInteractionRelease);
	m_selectionComponent = selectionComponentPtr;

	m_dragComponent.reset();
	m_dragOrigin = glm::vec3(0.f);
}

void GizmoRotateComponent::dispose()
{
	MikanComponent::dispose();

	SelectionComponentPtr selectionComponentPtr = m_selectionComponent.lock();
	selectionComponentPtr->OnInteractionRayOverlapEnter -= MakeDelegate(this, &GizmoRotateComponent::onInteractionRayOverlapEnter);
	selectionComponentPtr->OnInteractionRayOverlapExit -= MakeDelegate(this, &GizmoRotateComponent::onInteractionRayOverlapExit);
	selectionComponentPtr->OnInteractionGrab -= MakeDelegate(this, &GizmoRotateComponent::onInteractionGrab);
	selectionComponentPtr->OnInteractionMove -= MakeDelegate(this, &GizmoRotateComponent::onInteractionMove);
	selectionComponentPtr->OnInteractionRelease -= MakeDelegate(this, &GizmoRotateComponent::onInteractionRelease);
}

glm::vec3 GizmoRotateComponent::getColliderColor(
	DiskColliderComponentWeakPtr colliderPtr,
	const glm::vec3& defaultColor)
{
	if (colliderPtr.lock() == m_dragComponent.lock())
		return Colors::Yellow;
	else if (colliderPtr.lock() == m_dragComponent.lock())
		return Colors::LightGray;
	else
		return Colors::DarkGray;
}

static void drawRotateDiscHandle(DiskColliderComponentWeakPtr colliderWeakPtr, const glm::vec3 color)
{
	DiskColliderComponentPtr collidePtr = colliderWeakPtr.lock();

	//const glm::mat4 xform = collidePtr->getWorldTransform();
	//const float radius = collidePtr->getRadius();

	//drawTransformedCircle(xform, radius, color);
}

void GizmoRotateComponent::update()
{
	MikanComponent::update();

	if (m_bEnabled)
	{
		drawRotateDiscHandle(m_xAxisHandle, getColliderColor(m_xAxisHandle, Colors::Red));
		drawRotateDiscHandle(m_yAxisHandle, getColliderColor(m_yAxisHandle, Colors::Green));
		drawRotateDiscHandle(m_zAxisHandle, getColliderColor(m_zAxisHandle, Colors::Blue));
	}
}

void GizmoRotateComponent::setEnabled(bool bEnabled)
{
	if (m_bEnabled != bEnabled)
	{
		m_xAxisHandle.lock()->setEnabled(bEnabled);
		m_yAxisHandle.lock()->setEnabled(bEnabled);
		m_zAxisHandle.lock()->setEnabled(bEnabled);
		m_bEnabled = bEnabled;
	}
}

void GizmoRotateComponent::onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult)
{
	m_hoverComponent = hitResult.hitComponent;
}

void GizmoRotateComponent::onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult)
{
	m_hoverComponent.reset();
}

void GizmoRotateComponent::onInteractionGrab(const ColliderRaycastHitResult& hitResult)
{
	m_dragComponent = hitResult.hitComponent;
	m_dragOrigin = hitResult.hitLocation;
}

void GizmoRotateComponent::onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	ColliderComponentPtr dragColliderPtr = m_dragComponent.lock();

	const glm::mat4 centerXform = dragColliderPtr->getWorldTransform();
	const glm::vec3 origin = glm_mat4_position(centerXform);
	const glm::vec3 xAxis = glm_mat4_forward(centerXform);
	const glm::vec3 yAxis = glm_mat4_up(centerXform);
	const glm::vec3 zAxis = glm_mat4_right(centerXform);

	float int_time = 0.f;
	glm::vec3 int_point = m_dragOrigin;
	bool has_int = false;

	glm::quat newRotation;

	// X Axis drag
	if (dragColliderPtr == m_xAxisHandle.lock())
	{
		has_int = glm_intersect_plane_with_ray(
			origin, xAxis,
			rayOrigin, rayDir,
			int_time, int_point);
	}
	// Y Axis drag
	else if (dragColliderPtr == m_yAxisHandle.lock())
	{
		has_int = glm_intersect_plane_with_ray(
			origin, yAxis,
			rayOrigin, rayDir,
			int_time, int_point);
	}
	// Z Axis drag
	else if (dragColliderPtr == m_zAxisHandle.lock())
	{
		has_int = glm_intersect_plane_with_ray(
			origin, zAxis,
			rayOrigin, rayDir,
			int_time, int_point);
	}

	if (has_int)
	{
		requestRotation(newRotation);
	}
}

void GizmoRotateComponent::onInteractionRelease()
{
	m_dragComponent.reset();
}

void GizmoRotateComponent::requestRotation(const glm::quat& objectSpaceRotation)
{
	if (OnRotateRequested)
		OnRotateRequested(objectSpaceRotation);
}