#pragma once

#include "Colors.h"
#include "DiskColliderComponent.h"
#include "GizmoTransformComponent.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "GizmoRotateComponent.h"
#include "MikanObject.h"
#include "MathGLM.h"
#include "SelectionComponent.h"
#include "TextStyle.h"

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
	m_dragAngle = 0.f;
	m_worldSpaceDragBasis = glm::mat4(1.f);
	m_worldSpaceDragStart = glm::vec3(0.f);
	m_worldSpaceRotationAxis = glm::vec3(0.f);
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
		return defaultColor;
}

bool GizmoRotateComponent::getColliderRotationAxis(
	ColliderComponentWeakPtr colliderWeakPtr,
	glm::vec3& outWorldSpaceOrigin,
	glm::vec3& outWorldSpaceAxis)
{
	ColliderComponentPtr dragColliderPtr = colliderWeakPtr.lock();

	if (dragColliderPtr)
	{
		const glm::mat4 centerXform = dragColliderPtr->getWorldTransform();
		const glm::vec3 yAxis = glm_mat4_get_y_axis(centerXform);

		outWorldSpaceOrigin = glm_mat4_get_position(centerXform);
		outWorldSpaceAxis = yAxis;

		return true;
	}

	return false;
}

static constexpr float k_innerRingScale = 0.85f;

static void drawRotateDiscHandle(DiskColliderComponentWeakPtr colliderWeakPtr, const glm::vec3 color)
{
	DiskColliderComponentPtr collidePtr = colliderWeakPtr.lock();
	IMkGraphicsContext* graphicsContext = collidePtr->getGraphicsContext();

	const glm::mat4 xform = collidePtr->getWorldTransform();
	const float radius = collidePtr->getRadius();

	drawTransformedCircle(graphicsContext, xform, radius, color, GizmoTransformComponent::k_gizmoCircleSegments);
	drawTransformedCircle(graphicsContext, xform, radius * k_innerRingScale, color, GizmoTransformComponent::k_gizmoCircleSegments);
}

void GizmoRotateComponent::customRender()
{
	if (m_bEnabled)
	{
		ColliderComponentPtr dragComponentPtr = m_dragComponent.lock();

		if (!dragComponentPtr)
		{
			drawRotateDiscHandle(m_xAxisHandle, getColliderColor(m_xAxisHandle, Colors::Red));
			drawRotateDiscHandle(m_yAxisHandle, getColliderColor(m_yAxisHandle, Colors::Green));
			drawRotateDiscHandle(m_zAxisHandle, getColliderColor(m_zAxisHandle, Colors::Blue));
		}
		else
		{
			// Only draw the active handle while dragging
			if (dragComponentPtr == m_xAxisHandle.lock())
				drawRotateDiscHandle(m_xAxisHandle, Colors::Yellow);
			else if (dragComponentPtr == m_yAxisHandle.lock())
				drawRotateDiscHandle(m_yAxisHandle, Colors::Yellow);
			else if (dragComponentPtr == m_zAxisHandle.lock())
				drawRotateDiscHandle(m_zAxisHandle, Colors::Yellow);
		}
		if (dragComponentPtr)
		{
			IMkGraphicsContext* graphicsContext = dragComponentPtr->getGraphicsContext();
			auto diskComponentPtr = std::static_pointer_cast<DiskColliderComponent>(dragComponentPtr);
			const float radius = diskComponentPtr->getRadius();

			drawTransformedSpiralArc(
				graphicsContext,
				m_worldSpaceDragBasis, radius, 0.05f, m_dragAngle, Colors::Yellow,
				GizmoTransformComponent::k_gizmoCircleSegments);

			const glm::vec3 center = glm_mat4_get_position(diskComponentPtr->getWorldTransform());
			const float angleDeg = glm::degrees(m_dragAngle);
			TextStyle style = getDefaultTextStyle();

			const glm::vec3 labelPos = center + glm_mat4_get_x_axis(m_worldSpaceDragBasis) * radius * 1.5f;
			drawTextAtWorldPosition(graphicsContext, style, labelPos, L"%.1f\u00b0", angleDeg);
		}
	}
}

void GizmoRotateComponent::updateColliderScales(float displayScale)
{
	const float R = GizmoTransformComponent::k_gizmoBaseRadius * displayScale;
	if (auto h = m_xAxisHandle.lock()) h->setRadius(R);
	if (auto h = m_yAxisHandle.lock()) h->setRadius(R);
	if (auto h = m_zAxisHandle.lock()) h->setRadius(R);
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
	glm::vec3 worldSpaceRotationOrigin;
	if (getColliderRotationAxis(
		hitResult.hitComponent, 
		worldSpaceRotationOrigin, m_worldSpaceRotationAxis))
	{
		// Drag Basis is:
		// * Centered at rotation origin
		// * Looking at the drag start location (along +X)
		// * Point up along the rotation axis (along +Y)
		// * Positive drag angle along +Z
		const glm::vec3 x_axis = glm::normalize(hitResult.hitLocation - worldSpaceRotationOrigin);
		const glm::vec3 z_axis = glm::normalize(glm::cross(m_worldSpaceRotationAxis, x_axis));
		const glm::vec3 y_axis = glm::normalize(glm::cross(z_axis, x_axis));

		m_worldSpaceDragBasis = 
			glm::mat4(
				glm::vec4(x_axis, 0.f), 
				glm::vec4(y_axis, 0.f), 
				glm::vec4(z_axis, 0.f), 
				glm::vec4(worldSpaceRotationOrigin, 1.f));
		//glm::lookAt(worldSpaceRotationOrigin, hitResult.hitLocation, m_worldSpaceRotationAxis);
		m_worldSpaceDragStart = hitResult.hitLocation;

		m_dragComponent = hitResult.hitComponent;
		m_dragAngle= 0.f;

		// Disable the non-active handles so they don't interfere during drag
		ColliderComponentPtr dragPtr = m_dragComponent.lock();
		if (auto h = m_xAxisHandle.lock()) h->setEnabled(dragPtr == h);
		if (auto h = m_yAxisHandle.lock()) h->setEnabled(dragPtr == h);
		if (auto h = m_zAxisHandle.lock()) h->setEnabled(dragPtr == h);
	}
}

void GizmoRotateComponent::onInteractionMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	if (m_dragComponent.lock() != nullptr)
	{
		glm::vec3 worldSpaceDragPosDir = glm_mat4_get_z_axis(m_worldSpaceDragBasis);
		float dragRayTime = 0.f;
		glm::vec3 dragRayPoint = m_worldSpaceDragStart;

		if (glm_closest_point_on_ray_to_ray(
			m_worldSpaceDragStart, worldSpaceDragPosDir,
			rayOrigin, rayDir,
			dragRayTime, dragRayPoint))
		{
			const float newDragAngle = dragRayTime * k_dragAngleFactor;
			const float dragAngleDelta = newDragAngle - m_dragAngle;
			const glm::quat deltaRotation = glm::angleAxis(dragAngleDelta, m_worldSpaceRotationAxis);

			requestRotation(deltaRotation);
			m_dragAngle = newDragAngle;
		}
	}
}

void GizmoRotateComponent::onInteractionRelease()
{
	m_dragComponent.reset();
	m_dragAngle = 0.f;
	m_worldSpaceDragBasis = glm::mat4(1.f);
	m_worldSpaceDragStart = glm::vec3(0.f);
	m_worldSpaceRotationAxis= glm::vec3(0.f);

	// Re-enable all handles now that the drag is finished
	if (m_bEnabled)
	{
		if (auto h = m_xAxisHandle.lock()) h->setEnabled(true);
		if (auto h = m_yAxisHandle.lock()) h->setEnabled(true);
		if (auto h = m_zAxisHandle.lock()) h->setEnabled(true);
	}
}

void GizmoRotateComponent::requestRotation(const glm::quat& worldSpaceRotation)
{
	if (OnRotateRequested)
		OnRotateRequested(worldSpaceRotation);
}