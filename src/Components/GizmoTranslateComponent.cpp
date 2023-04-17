#pragma once

#include "BoxColliderComponent.h"
#include "GizmoTranslateComponent.h"
#include "SelectionComponent.h"
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
	selectionComponentPtr->OnInteractionRayPress += MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayPress);
	selectionComponentPtr->OnInteractionRayRelease += MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayRelease);
	m_selectionComponent= selectionComponentPtr;
}

void GizmoTranslateComponent::dispose()
{
	MikanComponent::dispose();

	SelectionComponentPtr selectionComponentPtr = m_selectionComponent.lock();
	selectionComponentPtr->OnInteractionRayOverlapEnter -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapEnter);
	selectionComponentPtr->OnInteractionRayOverlapExit -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayOverlapExit);
	selectionComponentPtr->OnInteractionRayPress -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayPress);
	selectionComponentPtr->OnInteractionRayRelease -= MakeDelegate(this, &GizmoTranslateComponent::onInteractionRayRelease);
}

void GizmoTranslateComponent::update()
{
	MikanComponent::update();
	if (m_bEnabled)
		return;
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

void GizmoTranslateComponent::onInteractionRayPress(const ColliderRaycastHitResult& hitResult, int button)
{
	if (button == SDL_BUTTON_LEFT)
	{
		m_dragComponent = hitResult.hitComponent;
	}
}

void GizmoTranslateComponent::onInteractionRayRelease(const ColliderRaycastHitResult& hitResult, int button)
{
	if (button == SDL_BUTTON_LEFT)
	{
		m_dragComponent.reset();
	}
}