#include "MikanObjectSystem.h"
#include "MikanObject.h"

MikanObjectSystem::MikanObjectSystem()
{

}

MikanObjectSystem::~MikanObjectSystem()
{
	dispose();
}

void MikanObjectSystem::init()
{
}

void MikanObjectSystem::dispose()
{
	m_objects.clear();
}

void MikanObjectSystem::update()
{
	for (MikanObjectPtr object : m_objects)
	{
		object->update();
	}
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
		auto it = std::find(m_objects.begin(), m_objects.end(), objectPtr);
		if (it != m_objects.end())
		{
			if (OnObjectRemoved)
				OnObjectRemoved(*this, *objectPtr.get());

			m_objects.erase(it);
		}
	}
}
