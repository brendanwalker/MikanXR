#include "MikanComponent.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"

MikanComponent::MikanComponent(MikanObjectWeakPtr owner)
	: m_ownerObject(owner)
{
}

MikanComponent::~MikanComponent()
{

}

void MikanComponent::init()
{
	if (m_bWantsUpdate)
	{
		getOwnerObject()->getOwnerSystem()->onUpdate+= MakeDelegate(this, &MikanComponent::update);
	}

	if (m_bWantsCustomRender)
	{
		getOwnerObject()->getOwnerSystem()->onCustomRender += MakeDelegate(this, &MikanComponent::customRender);
	}

	m_bIsInitialized= true;
}

void MikanComponent::dispose()
{
	if (m_bWantsUpdate)
	{
		getOwnerObject()->getOwnerSystem()->onUpdate -= MakeDelegate(this, &MikanComponent::update);
	}

	if (m_bWantsCustomRender)
	{
		getOwnerObject()->getOwnerSystem()->onCustomRender -= MakeDelegate(this, &MikanComponent::customRender);
	}

	m_bIsDisposed= true;
}