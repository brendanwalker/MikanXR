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
	for (MikanComponentPtr component : m_components)
	{
		component->init();
	}
}

void MikanObject::dispose()
{
	for (MikanComponentPtr component : m_components)
	{
		component->dispose();
	}

	m_components.clear();
}

void MikanObject::update()
{
	for (MikanComponentPtr component : m_components)
	{
		component->update();
	}
}