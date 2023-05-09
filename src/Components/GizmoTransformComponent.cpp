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
	m_bWantsCustomRender= true;
}

void GizmoTransformComponent::init()
{
	SceneComponent::init();

	MikanObjectPtr owner= getOwnerObject();

	m_translateComponent = owner->getComponentOfType<GizmoTranslateComponent>();
	m_rotateComponent = owner->getComponentOfType<GizmoRotateComponent>();
	m_scaleComponent = owner->getComponentOfType<GizmoScaleComponent>();
}

void GizmoTransformComponent::bindInput()
{
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_t)->OnKeyPressed +=
		MakeDelegate(this, &GizmoTransformComponent::selectTranslateMode);
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_r)->OnKeyPressed +=
		MakeDelegate(this, &GizmoTransformComponent::selectRotateMode);
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_y)->OnKeyPressed +=
		MakeDelegate(this, &GizmoTransformComponent::selectScaleMode);
}

void GizmoTransformComponent::customRender()
{
	GizmoTranslateComponentPtr translatePtr = m_translateComponent.lock();
	GizmoRotateComponentPtr rotatePtr = m_rotateComponent.lock();
	GizmoScaleComponentPtr scalePtr = m_scaleComponent.lock();

	switch (m_gizmoMode)
	{
	case eGizmoMode::rotate:
		rotatePtr->customRender();
		break;
	case eGizmoMode::translate:
		translatePtr->customRender();
		break;
	case eGizmoMode::scale:
		scalePtr->customRender();
		break;
	}
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

SceneComponentPtr GizmoTransformComponent::getTransformTarget() const
{
	return m_targetSceneComponent.lock();
}

void GizmoTransformComponent::setTransformTarget(SceneComponentPtr sceneComponentTarget)
{
	// Remember the new transform target
	m_targetSceneComponent= sceneComponentTarget;

	eGizmoMode oldGizmoMode = getGizmoMode();
	eGizmoMode newGizmoMode = eGizmoMode::none;

	// Default to a transform mode gizmo if the gizmo wasn't active before
	if (oldGizmoMode != eGizmoMode::none)
		newGizmoMode = oldGizmoMode;
	else
		newGizmoMode = eGizmoMode::translate;

	// Update the desired gizmo state
	setGizmoMode(newGizmoMode);

	// Snap the gizmo to the target scene component
	setWorldTransform(sceneComponentTarget->getWorldTransform());
}

void GizmoTransformComponent::clearTransformTarget()
{
	m_targetSceneComponent.reset();
	setGizmoMode(eGizmoMode::none);
	setWorldTransform(glm::mat4(1.f));
}

void GizmoTransformComponent::applyTransformToTarget()
{
	SceneComponentPtr sceneComponentTarget= m_targetSceneComponent.lock();

	if (sceneComponentTarget)
	{
		sceneComponentTarget->setWorldTransform(getWorldTransform());
	}
}