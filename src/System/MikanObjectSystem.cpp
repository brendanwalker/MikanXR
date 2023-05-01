#include "MikanObjectSystem.h"
#include "MikanObject.h"

#include "assert.h"

MikanObjectSystem::MikanObjectSystem()
{

}

MikanObjectSystem::~MikanObjectSystem()
{
	assert(m_objects.empty());
}

void MikanObjectSystem::init()
{
}

void MikanObjectSystem::dispose()
{
	for (MikanObjectPtr objectPtr : m_objects)
	{
		objectPtr->dispose();
	}
	m_objects.clear();
}

void MikanObjectSystem::update()
{
	if (onUpdate)
		onUpdate();
}

void MikanObjectSystem::customRender()
{
	if (onCustomRender)
		onCustomRender();
}

MikanObjectWeakPtr MikanObjectSystem::newObject()
{
	MikanObjectPtr objectPtr = std::make_shared<MikanObject>(shared_from_this());
	m_objects.push_back(objectPtr);

	if (OnObjectAdded)
		OnObjectAdded(*this, *objectPtr.get());

	return objectPtr;
}

void MikanObjectSystem::deleteObject(MikanObjectWeakPtr objectWeakPtr)
{
	MikanObjectPtr objectPtr= objectWeakPtr.lock();

	if (objectPtr)
	{
		objectPtr->dispose();

		auto it = std::find(m_objects.begin(), m_objects.end(), objectPtr);
		if (it != m_objects.end())
		{
			if (OnObjectRemoved)
				OnObjectRemoved(*this, *objectPtr.get());

			m_objects.erase(it);
		}
	}
}
