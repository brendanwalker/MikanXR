#pragma once

#include "DiskColliderComponent.h"
#include "GizmoRotateComponent.h"
#include "MikanObject.h"

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
}

void GizmoRotateComponent::update()
{
	MikanComponent::update();
}