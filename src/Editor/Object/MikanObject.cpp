#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "MikanComponent.h"

#include "assert.h"

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

	MikanObjectSystemPtr objectSystem= m_ownerObjectSystem.lock();
	if (objectSystem->OnObjectInitialized)
		objectSystem->OnObjectInitialized(objectSystem, shared_from_this());
}

void MikanObject::dispose()
{
	for (MikanComponentPtr component : m_components)
	{
		component->dispose();
	}

	MikanObjectSystemPtr objectSystem = m_ownerObjectSystem.lock();
	if (objectSystem->OnObjectDisposed)
		objectSystem->OnObjectDisposed(objectSystem, shared_from_this());

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