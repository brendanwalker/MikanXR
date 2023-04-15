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