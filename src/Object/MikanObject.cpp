#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "MikanComponent.h"

#include "assert.h"

MulticastDelegate<void(MikanObject&)> ObjectEvents::OnObjectInitialized;
MulticastDelegate<void(const MikanObject&)> ObjectEvents::OnObjectDisposed;

MikanObject::MikanObject(MikanObjectSystemWeakPtr ownerSystemPtr)
	: m_ownerObjectSystem(ownerSystemPtr)
{
}

MikanObject::~MikanObject()
{
	// dispose should have been called already
	assert(m_components.empty());
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
	m_ownerObjectSystem.reset();
}

void MikanObject::deleteSelfConfig()
{
	MikanObjectSystemPtr ownerSystem= getOwnerSystem();
	if (ownerSystem != nullptr)
	{
		ownerSystem->deleteObjectConfig(shared_from_this());
	}
}