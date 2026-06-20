#include "RGBSpotLightSystem.h"
#include "DMXObjectSystem.h"
#include "IDMXManager.h"
#include "MikanObject.h"
#include "SelectionComponent.h"

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

void RGBSpotLightSystem::additionalComponentFactory(
	MikanObjectPtr ownerComponentObject,
	ComponentDefinitionPtr componentDefinition)
{
	// Add a selection component so the mesh can be clicked in the viewport
	ownerComponentObject->addComponent<SelectionComponent>();

	// Build mesh + collider components for the spot light model
	RGBSpotLightComponentPtr lightComponentPtr=
		ownerComponentObject->getComponentOfType<RGBSpotLightComponent>();
	if (lightComponentPtr)
		lightComponentPtr->rebuildMeshComponents();
}

void RGBSpotLightSystem::update(float deltaSeconds)
{
	Super::update(deltaSeconds);

	IDMXManager* dmxManager= getObjectSystemOfType<DMXObjectSystem>()->getDMXManager();
	if (!dmxManager || !dmxManager->getIsRunning())
		return;

	for (const auto& [lightId, componentWeakPtr] : Super::getComponentMap())
	{
		RGBSpotLightComponentPtr component= componentWeakPtr.lock();

		if (component)
			component->sendDMXData(dmxManager);
	}
}