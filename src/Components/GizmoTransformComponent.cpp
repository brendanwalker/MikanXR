#pragma once

#include "GizmoTransformComponent.h"
#include "GizmoTranslateComponent.h"
#include "GizmoRotateComponent.h"
#include "GizmoScaleComponent.h"
#include "MikanObject.h"

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