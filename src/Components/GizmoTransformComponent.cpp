#pragma once

#include "GizmoTransformComponent.h"
#include "GizmoTranslateComponent.h"
#include "GizmoRotateComponent.h"
#include "GizmoScaleComponent.h"
#include "InputManager.h"
#include "MikanObject.h"

#include "SDL_keycode.h"

GizmoTransformComponent::GizmoTransformComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
}

void GizmoTransformComponent::init()
{
	SceneComponent::init();

	MikanObjectPtr owner= getOwnerObject();

	m_translateComponent = owner->getComponentOfType<GizmoTranslateComponent>();
	m_rotateComponent = owner->getComponentOfType<GizmoRotateComponent>();
	m_scaleComponent = owner->getComponentOfType<GizmoScaleComponent>();

	// Following Unity gizmo hotkey defaults
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_w)->OnKeyPressed +=
		MakeDelegate(this, &GizmoTransformComponent::selectTranslateMode);
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_e)->OnKeyPressed +=
		MakeDelegate(this, &GizmoTransformComponent::selectRotateMode);
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_r)->OnKeyPressed +=
		MakeDelegate(this, &GizmoTransformComponent::selectScaleMode);
}

void GizmoTransformComponent::dispose()
{
	SceneComponent::dispose();
}

void GizmoTransformComponent::setGizmoMode(eGizmoMode newMode)
{
	if (newMode == m_gizmoMode)
		return;

	GizmoTranslateComponentPtr translatePtr= m_translateComponent.lock();
	GizmoRotateComponentPtr rotatePtr= m_rotateComponent.lock();
	GizmoScaleComponentPtr scalePtr= m_scaleComponent.lock();

	translatePtr->setEnabled(false);
	rotatePtr->setEnabled(false);
	scalePtr->setEnabled(false);

	switch (newMode)
	{
	case eGizmoMode::rotate:
		rotatePtr->setEnabled(true);
		break;
	case eGizmoMode::translate:
		translatePtr->setEnabled(true);
		break;
	case eGizmoMode::scale:
		scalePtr->setEnabled(true);
		break;
	}

	m_gizmoMode= newMode;
}

void GizmoTransformComponent::selectTranslateMode()
{
	if (m_gizmoMode != eGizmoMode::none)
		setGizmoMode(eGizmoMode::translate);
}

void GizmoTransformComponent::selectRotateMode()
{
	if (m_gizmoMode != eGizmoMode::none)
		setGizmoMode(eGizmoMode::rotate);
}

void GizmoTransformComponent::selectScaleMode()
{
	if (m_gizmoMode != eGizmoMode::none)
		setGizmoMode(eGizmoMode::translate);
}