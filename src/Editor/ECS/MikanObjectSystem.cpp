#include "MikanObjectSystem.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "MikanVariantTypes.h"

#include "assert.h"

MikanObjectSystem::MikanObjectSystem(ProjectManager* ownerObjectSystem)
	: m_ownerObjectSystemManager(ownerObjectSystem)
{

}

MikanObjectSystem::~MikanObjectSystem()
{
	assert(m_objects.empty());
}

bool MikanObjectSystem::init()
{
	// Assign this system as owner to it's corresponding definition
	auto systemConfig= getObjectSystemConfig();
	if (systemConfig)
	{
		systemConfig->setOwnerSystem(shared_from_this());
	}

	return true;
}

void MikanObjectSystem::dispose()
{
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

ProjectConfigPtr MikanObjectSystem::getProjectConfig() const
{
	return m_ownerObjectSystemManager->getProjectConfig();
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