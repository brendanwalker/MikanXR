#pragma once

#include "BoxColliderComponent.h"
#include "GizmoTranslateComponent.h"
#include "MikanObject.h"

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
}

void GizmoTranslateComponent::update()
{
	MikanComponent::update();
}

void GizmoTranslateComponent::setEnabled(bool bEnabled)
{
	m_centerHandle.lock()->setEnabled(bEnabled);
	m_xyHandle.lock()->setEnabled(bEnabled);
	m_xzHandle.lock()->setEnabled(bEnabled);
	m_yzHandle.lock()->setEnabled(bEnabled);
	m_xAxisHandle.lock()->setEnabled(bEnabled);
	m_yAxisHandle.lock()->setEnabled(bEnabled);
	m_zAxisHandle.lock()->setEnabled(bEnabled);
}