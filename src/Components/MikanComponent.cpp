#include "MikanComponent.h"
#include "MikanObject.h"

MikanComponent::MikanComponent(MikanObjectWeakPtr owner)
	: m_ownerObject(owner)
{
}

MikanComponent::MikanComponent(
	const std::string& name, 
	MikanObjectWeakPtr owner)
	: m_name(name)
	, m_ownerObject(owner)
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
