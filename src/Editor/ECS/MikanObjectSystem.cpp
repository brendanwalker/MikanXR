#include "MikanObjectSystem.h"
#include "MikanObject.h"

#include "assert.h"

MikanObjectSystem::MikanObjectSystem(ObjectSystemManager* ownerObjectSystem)
	: m_ownerObjectSystemManager(ownerObjectSystem)
{

}

MikanObjectSystem::~MikanObjectSystem()
{
	assert(m_objects.empty());
}

bool MikanObjectSystem::init()
{
	MikanObjectSystemDefinitionPtr definition = getObjectSystemConfig();
	if (definition)
	{
		definition->OnMarkedDirty += MakeDelegate(this, &MikanObjectSystem::onDefinitionMarkedDirty);
	}

	return true;
}

void MikanObjectSystem::dispose()
{
	MikanObjectSystemDefinitionPtr definition = getObjectSystemConfig();
	if (definition)
	{
		definition->OnMarkedDirty -= MakeDelegate(this, &MikanObjectSystem::onDefinitionMarkedDirty);
	}

	deleteAllObjects();
}

void MikanObjectSystem::update(float deltaSeconds)
{
	if (onUpdate)
		onUpdate(deltaSeconds);
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

void MikanObjectSystem::deleteAllObjects()
{
	for (MikanObjectPtr objectPtr : m_objects)
	{
		objectPtr->dispose();
	}
	m_objects.clear();
}

void MikanObjectSystem::onDefinitionMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (OnPropertyChanged)
	{
		// TODO: Only notify for property names that are actually exposed in getPropertyNames()
		for (const std::string& changedPropertyName : changedPropertySet.getSet())
		{
			OnPropertyChanged(changedPropertyName);
		}
	}
}