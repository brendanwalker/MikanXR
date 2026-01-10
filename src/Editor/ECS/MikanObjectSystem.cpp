#include "MikanObjectSystem.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "MikanVariantTypes.h"

#include "assert.h"

MikanObjectSystem::MikanObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: m_ownerObjectSystemManager(ownerObjectSystem)
{

}

MikanObjectSystem::~MikanObjectSystem()
{
	assert(m_objects.empty());
}

bool MikanObjectSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	m_definitionWeakPtr = definitionPtr;
	if (definitionPtr)
	{
		definitionPtr->setOwnerSystem(shared_from_this());
	}

	return true;
}

void MikanObjectSystem::dispose()
{
	deleteAllObjects();

	if (onDisposed)
	{
		onDisposed(this);
	}
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

ProjectConfigPtr MikanObjectSystem::getProjectConfig() const
{
	return getOwnerProjectManager()->getProjectConfig();
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

void MikanObjectSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<MikanObjectSystem>();
}