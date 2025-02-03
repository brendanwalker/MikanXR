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

bool MikanObjectSystem::init()
{
	return true;
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

MikanObjectPtr MikanObjectSystem::newObject()
{
	MikanObjectPtr objectPtr = std::make_shared<MikanObject>(shared_from_this());
	m_objects.push_back(objectPtr);

	return objectPtr;
}

void MikanObjectSystem::deleteObject(MikanObjectPtr objectPtr)
{
	if (objectPtr)
	{
		objectPtr->dispose();

		auto it = std::find(m_objects.begin(), m_objects.end(), objectPtr);
		if (it != m_objects.end())
		{
			m_objects.erase(it);
		}
	}
}
