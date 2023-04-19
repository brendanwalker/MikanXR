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

	SceneComponentPtr sceneComponentPtr= getOwnerObject()->getComponentOfType<SceneComponent>();
	sceneComponentPtr->OnTranformChaged += MakeDelegate(this, &StencilComponent::onSceneComponentTranformChaged);
	m_sceneComponent= sceneComponentPtr;
}

void StencilComponent::dispose()
{
	SceneComponentPtr sceneComponentPtr = m_sceneComponent.lock();
	sceneComponentPtr->OnTranformChaged -= MakeDelegate(this, &StencilComponent::onSceneComponentTranformChaged);
}

void StencilComponent::onSceneComponentTranformChaged(SceneComponentPtr sceneComponentPtr)
{
	// Update stencil transform properties
	setStencilWorldTransformProperty(sceneComponentPtr->getWorldTransform());
}