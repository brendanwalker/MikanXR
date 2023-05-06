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

static const float k_dragAngleFactor= k_real_two_pi; // 360 degrees / meter

GizmoRotateComponent::GizmoRotateComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{ }

void GizmoRotateComponent::init()
{
	MikanComponent::init();

	MikanObjectPtr owner = getOwnerObject();

	m_xAxisHandle = owner->getComponentOfTypeAndName<DiskColliderComponent>("xAxisRotateHandle");
	m_yAxisHandle = owner->getComponentOfTypeAndName<DiskColliderComponent>("yAxisRotateHandle");
	m_zAxisHandle = owner->getComponentOfTypeAndName<DiskColliderComponent>("zAxisRotateHandle");

	m_selectionComponent = owner->getComponentOfType<SelectionComponent>();

	m_dragComponent.reset();
	m_dragBasis = glm::mat4(1.f);
	m_dragStart = glm::vec3(0.f);
}

void GizmoRotateComponent::dispose()
{
	setEnabled(false);
	MikanComponent::dispose();
}

glm::vec3 GizmoRotateComponent::getColliderColor(
	DiskColliderComponentWeakPtr colliderPtr,
	const glm::vec3& defaultColor) const
{
	if (colliderPtr.lock() == m_dragComponent.lock())
		return Colors::Yellow;
	else if (colliderPtr.lock() == m_hoverComponent.lock())
		return Colors::LightGray;
	else
		return Colors::DarkGray;
}

bool GizmoRotateComponent::getColliderRotationAxis(
	ColliderComponentWeakPtr colliderWeakPtr,
	glm::vec3& outOrigin,
	glm::vec3& outAxis)
{
	ColliderComponentPtr dragColliderPtr = colliderWeakPtr.lock();

	if (dragColliderPtr)
	{
		const glm::mat4 centerXform = dragColliderPtr->getWorldTransform();
		const glm::vec3 xAxis = glm_mat4_get_x_axis(centerXform);
		const glm::vec3 yAxis = glm_mat4_get_y_axis(centerXform);
		const glm::vec3 zAxis = glm_mat4_get_z_axis(centerXform);

		outOrigin = glm_mat4_get_position(centerXform);

		if (dragColliderPtr == m_xAxisHandle.lock())
		{
			outAxis = xAxis;
			return true;
		}
		else if (dragColliderPtr == m_yAxisHandle.lock())
		{
			outAxis = yAxis;
			return true;
		}
		else if (dragColliderPtr == m_zAxisHandle.lock())
		{
			outAxis = zAxis;
			return true;
		}
	}

	return false;
}

static void drawRotateDiscHandle(DiskColliderComponentWeakPtr colliderWeakPtr, const glm::vec3 color)
{
	DiskColliderComponentPtr collidePtr = colliderWeakPtr.lock();

	const glm::mat4 xform = collidePtr->getWorldTransform();
	const float radius = collidePtr->getRadius();

	drawTransformedCircle(xform, radius, color);
}

void GizmoRotateComponent::customRender()
{
	if (m_bEnabled)
	{
		drawRotateDiscHandle(m_xAxisHandle, getColliderColor(m_xAxisHandle, Colors::Red));
		drawRotateDiscHandle(m_yAxisHandle, getColliderColor(m_yAxisHandle, Colors::Green));
		drawRotateDiscHandle(m_zAxisHandle, getColliderColor(m_zAxisHandle, Colors::Blue));

		ColliderComponentPtr dragComponentPtr = m_dragComponent.lock();
		if (dragComponentPtr)
		{
			auto diskComponentPtr = std::static_pointer_cast<DiskColliderComponent>(dragComponentPtr);
			const float radius = diskComponentPtr->getRadius();

			drawTransformedSpiralArc(m_dragBasis, radius, 0.05f, m_dragAngle, Colors::Yellow);
		}
	}
}

void GizmoRotateComponent::setEnabled(bool bEnabled)
{
	if (m_bEnabled != bEnabled)
	{
		SelectionComponentPtr selectionComponentPtr = m_selectionComponent.lock();

		if (bEnabled)
		{
			selectionComponentPtr->OnInteractionRayOverlapEnter += MakeDelegate(this, &GizmoRotateComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit += MakeDelegate(this, &GizmoRotateComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab += MakeDelegate(this, &GizmoRotateComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove += MakeDelegate(this, &GizmoRotateComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease += MakeDelegate(this, &GizmoRotateComponent::onInteractionRelease);
		}
		else
		{
			selectionComponentPtr->OnInteractionRayOverlapEnter -= MakeDelegate(this, &GizmoRotateComponent::onInteractionRayOverlapEnter);
			selectionComponentPtr->OnInteractionRayOverlapExit -= MakeDelegate(this, &GizmoRotateComponent::onInteractionRayOverlapExit);
			selectionComponentPtr->OnInteractionGrab -= MakeDelegate(this, &GizmoRotateComponent::onInteractionGrab);
			selectionComponentPtr->OnInteractionMove -= MakeDelegate(this, &GizmoRotateComponent::onInteractionMove);
			selectionComponentPtr->OnInteractionRelease -= MakeDelegate(this, &GizmoRotateComponent::onInteractionRelease);
		}

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
	glm::vec3 rotationOrigin;
	glm::vec3 rotationAxis;
	if (getColliderRotationAxis(hitResult.hitComponent, rotationOrigin, rotationAxis))
	{
		// Drag Basis is:
		// * Centered at rotation origin
		// * Looking at the drag start location (along +X)
		// * Point up along the rotation axis (along +Y)
		// * Positive drag angle along +Z
		m_dragBasis= glm::lookAt(rotationOrigin, hitResult.hitLocation, rotationAxis);
		m_dragComponent = hitResult.hitComponent;
		m_dragStart= hitResult.hitLocation;
		m_dragAngle= 0.f;
	}
}

void GizmoRotateComponent::onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	glm::vec3 rotationOrigin;
	glm::vec3 rotationAxis;
	if (getColliderRotationAxis(m_dragComponent, rotationOrigin, rotationAxis))
	{
		const glm::vec3 dragPositiveDir = glm_mat4_get_z_axis(m_dragBasis);

		float int_time = 0.f;
		glm::vec3 int_point = m_dragStart;

		if (glm_closest_point_on_ray_to_ray(
			m_dragStart, dragPositiveDir,
			rayOrigin, rayDir,
			int_time, int_point))
		{
			const float newDragAngle = int_time * k_dragAngleFactor;
			const float dragAngleDelta = newDragAngle - m_dragAngle;
			const glm::quat deltaRotation = glm::angleAxis(dragAngleDelta, rotationAxis);

			requestRotation(deltaRotation);
			m_dragAngle = newDragAngle;
		}
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