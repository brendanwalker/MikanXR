#include "MikanStencilComponent.h"
#include "MikanSceneComponent.h"
#include "MikanObject.h"

MikanStencilComponent::MikanStencilComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, StencilId(INVALID_MIKAN_ID)
	, ParentAnchorId(INVALID_MIKAN_ID)
	, IsDisabled(false)
	, StencilName("")
{
}

void MikanStencilComponent::init()
{
	MikanComponent::init();

	m_sceneComponent= getOwnerObject()->getComponentOfType<MikanSceneComponent>();
}