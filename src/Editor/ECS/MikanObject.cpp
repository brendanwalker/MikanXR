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
	assert(!m_bIsInitialized);

	for (MikanComponentPtr component : m_components)
	{
		component->init();
	}

	m_bIsInitialized = true;
}

void MikanObject::postInit()
{
	assert(!m_bIsPostInitialized);

	for (MikanComponentPtr component : m_components)
	{
		component->postInit();
	}

	m_bIsPostInitialized = true;
}

void MikanObject::dispose()
{
	assert(!m_bIsDisposed);

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

	m_bIsDisposed = true;
}