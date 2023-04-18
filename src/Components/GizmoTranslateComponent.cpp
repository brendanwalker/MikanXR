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

glm::vec3 GizmoTranslateComponent::getColliderColor(BoxColliderComponentWeakPtr colliderPtr)
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

void GizmoTranslateComponent::update()
{
	MikanComponent::update();
	
	if (m_bEnabled)
	{
		drawTranslationBoxHandle(m_centerHandle, getColliderColor(m_centerHandle));
		drawTranslationBoxHandle(m_xyHandle, getColliderColor(m_xyHandle));
		drawTranslationBoxHandle(m_xzHandle, getColliderColor(m_xzHandle));
		drawTranslationBoxHandle(m_yzHandle, getColliderColor(m_yzHandle));
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

	// Center handle drag
	if (dragColliderPtr == m_centerHandle.lock())
	{
		float ray_closest_time= 0.f;
		glm::vec3 ray_closest_point= glm::vec3(0.f);
		glm_closest_point_on_ray_to_point(
			rayOrigin, rayDir, m_dragOrigin,
			ray_closest_time, ray_closest_point);

		const glm::vec3 translation= ray_closest_point - m_dragOrigin;
		requestTranslation(translation);
	}
}

void GizmoTranslateComponent::onInteractionRelease()
{
	m_dragComponent.reset();
}

void GizmoTranslateComponent::requestTranslation(const glm::vec3& translation)
{
	if (OnTranslationRequested)
		OnTranslationRequested(translation);
}