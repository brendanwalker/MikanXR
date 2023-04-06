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

void MikanObjectSystem::addObject(MikanObjectPtr objectPtr)
{
	m_objects.push_back(objectPtr);
}

void MikanObjectSystem::removeObject(MikanObjectPtr objectPtr)
{
	auto it= std::find(m_objects.begin(), m_objects.end(), objectPtr);
	if (it != m_objects.end())
	{
		m_objects.erase(it);
	}
}
