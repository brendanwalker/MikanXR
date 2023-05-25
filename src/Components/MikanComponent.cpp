#include "MikanComponent.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"

MikanComponent::MikanComponent(MikanObjectWeakPtr owner)
	: m_ownerObject(owner)
{
}

void MikanComponent::init()
{
	if (m_bIsInitialized)
		return;

	MikanObjectSystemPtr objectSystemPtr= getOwnerObject()->getOwnerSystem();

	if (m_bWantsUpdate)
	{
		objectSystemPtr->onUpdate+= MakeDelegate(this, &MikanComponent::update);
	}

	if (m_bWantsCustomRender)
	{
		objectSystemPtr->onCustomRender += MakeDelegate(this, &MikanComponent::customRender);
	}

	m_bIsInitialized= true;

	if (objectSystemPtr->OnComponentInitialized)
	{
		objectSystemPtr->OnComponentInitialized(objectSystemPtr, shared_from_this());
	}
}

void MikanComponent::dispose()
{
	if (m_bIsDisposed)
		return;

	MikanObjectSystemPtr objectSystemPtr= getOwnerObject()->getOwnerSystem();

	if (objectSystemPtr->OnComponentDisposed)
	{
		objectSystemPtr->OnComponentDisposed(objectSystemPtr, shared_from_this());
	}

	if (m_bWantsUpdate)
	{
		objectSystemPtr->onUpdate -= MakeDelegate(this, &MikanComponent::update);
	}

	if (m_bWantsCustomRender)
	{
		objectSystemPtr->onCustomRender -= MakeDelegate(this, &MikanComponent::customRender);
	}

	m_bIsDisposed= true;
}