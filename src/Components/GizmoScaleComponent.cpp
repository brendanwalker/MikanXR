#pragma once

#include "BoxColliderComponent.h"
#include "GizmoScaleComponent.h"
#include "MikanObject.h"

GizmoScaleComponent::GizmoScaleComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{}

void GizmoScaleComponent::init()
{
	MikanComponent::init();

	MikanObjectPtr owner = getOwnerObject();

	m_centerHandle = owner->getComponentOfTypeAndName<BoxColliderComponent>("centerScaleHandle");
	m_xAxisHandle = owner->getComponentOfTypeAndName<BoxColliderComponent>("xAxisScaleHandle");
	m_yAxisHandle = owner->getComponentOfTypeAndName<BoxColliderComponent>("yAxisScaleHandle");
	m_zAxisHandle = owner->getComponentOfTypeAndName<BoxColliderComponent>("zAxisScaleHandle");
}

void GizmoScaleComponent::update()
{
	MikanComponent::update();
}

void GizmoScaleComponent::setEnabled(bool bEnabled)
{
	m_centerHandle.lock()->setEnabled(bEnabled);
	m_xAxisHandle.lock()->setEnabled(bEnabled);
	m_yAxisHandle.lock()->setEnabled(bEnabled);
	m_zAxisHandle.lock()->setEnabled(bEnabled);
}