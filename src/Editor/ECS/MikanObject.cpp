#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "MikanComponent.h"

#include "assert.h"

MikanObject::MikanObject(MikanObjectSystemWeakPtr ownerSystemPtr)
	: m_ownerObjectSystemManager(ownerSystemPtr)
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

	MikanObjectSystemPtr objectSystem= m_ownerObjectSystemManager.lock();
	if (objectSystem->OnObjectInitialized)
		objectSystem->OnObjectInitialized(objectSystem, shared_from_this());
}

void MikanObject::postInit()
{
	for (MikanComponentPtr component : m_components)
	{
		component->postInit();
	}
}

void MikanObject::dispose()
{
	for (MikanComponentPtr component : m_components)
	{
		component->dispose();
	}

	if (auto objectSystem = m_ownerObjectSystemManager.lock())
	{
		if (objectSystem->OnObjectDisposed)
			objectSystem->OnObjectDisposed(objectSystem, shared_from_this());
	}

	m_components.clear();
	m_ownerObjectSystemManager.reset();
}