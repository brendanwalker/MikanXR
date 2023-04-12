#include "StencilComponent.h"
#include "SceneComponent.h"
#include "MikanObject.h"

StencilComponent::StencilComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, StencilId(INVALID_MIKAN_ID)
	, ParentAnchorId(INVALID_MIKAN_ID)
	, IsDisabled(false)
	, StencilName("")
{
}

void StencilComponent::init()
{
	MikanComponent::init();

	m_sceneComponent= getOwnerObject()->getComponentOfType<SceneComponent>();
}