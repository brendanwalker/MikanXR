#include "MikanObject.h"

MikanObject::MikanObject()
{

}

MikanObject::~MikanObject()
{
	dispose();
}

void MikanObject::init()
{
	for (MikanComponent* component : m_components)
	{
		component->init();
	}
}

void MikanObject::dispose()
{
	for (MikanComponent* component : m_components)
	{
		component->dispose();
		delete component;
	}

	m_components.clear();
}

void MikanObject::update()
{
	for (MikanComponent* component : m_components)
	{
		component->update();
	}
}