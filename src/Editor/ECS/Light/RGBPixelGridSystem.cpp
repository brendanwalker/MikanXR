#include "RGBPixelGridSystem.h"
#include "DMXObjectSystem.h"
#include "IDMXManager.h"

// -- RGBPixelGridSystemDefinition -----
RGBPixelGridSystemDefinition::RGBPixelGridSystemDefinition(const std::string& configName,
														   IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- RGBPixelGridSystem -----
RGBPixelGridSystem::RGBPixelGridSystem(ProjectManagerPtr ownerProjectManager)
	: Super::MikanTypedObjectSystem(ownerProjectManager)
{
}

void RGBPixelGridSystem::update(float deltaSeconds)
{
	Super::update(deltaSeconds);

	IDMXManager* dmxManager= getObjectSystemOfType<DMXObjectSystem>()->getDMXManager();
	if (!dmxManager || !dmxManager->getIsRunning())
		return;

	for (const auto& [gridId, componentWeakPtr] : Super::getComponentMap())
	{
		RGBPixelGridComponentPtr component= componentWeakPtr.lock();
		if (component)
			component->sendDMXData(dmxManager);
	}
}