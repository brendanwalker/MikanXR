#include "MikanComponent.h"
#include "MikanObject.h"

MikanComponent::MikanComponent(MikanObjectWeakPtr owner)
	: m_ownerObject(owner)
{
}

MikanComponent::~MikanComponent()
{

}

void MikanComponent::init()
{

}

void MikanComponent::update()
{

}

void MikanComponent::dispose()
{

}

void MikanComponent::notifyComponentPropertyChanged(const ComponentProperty& property)
{
	if (OnComponentPropertyChanged)
		OnComponentPropertyChanged(property);
}