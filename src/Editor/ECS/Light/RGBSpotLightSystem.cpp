#include "RGBSpotLightSystem.h"
#include "DMXObjectSystem.h"
#include "IDMXManager.h"

// -- RGBSpotLightSystemDefinition -----
RGBSpotLightSystemDefinition::RGBSpotLightSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- RGBSpotLightSystem -----
RGBSpotLightSystem::RGBSpotLightSystem(ProjectManagerPtr ownerProjectManager)
	: Super::MikanTypedObjectSystem(ownerProjectManager)
{
}

void RGBSpotLightSystem::update(float deltaSeconds)
{
	Super::update(deltaSeconds);

	IDMXManager* dmxManager = getObjectSystemOfType<DMXObjectSystem>()->getDMXManager();
	if (!dmxManager || !dmxManager->getIsRunning())
		return;

	for (const auto& [lightId, componentWeakPtr] : Super::getComponentMap())
	{
		RGBSpotLightComponentPtr component = componentWeakPtr.lock();

		if (component)
			component->sendDMXData(dmxManager);
	}
}