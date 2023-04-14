#include "MikanObject.h"
#include "MikanComponent.h"

MulticastDelegate<void(MikanObject&)> ObjectEvents::OnObjectInitialized;
MulticastDelegate<void(const MikanObject&)> ObjectEvents::OnObjectDisposed;

MikanObject::MikanObject(MikanObjectSystemWeakPtr ownerSystemPtr)
	: m_ownerObjectSystem(ownerSystemPtr)
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

	if (ObjectEvents::OnObjectInitialized)
		ObjectEvents::OnObjectInitialized(*this);
}

void MikanObject::dispose()
{
	if (ObjectEvents::OnObjectDisposed)
		ObjectEvents::OnObjectDisposed(*this);

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