#include "VRTrackingVolumeSystem.h"
#include "Logger.h"
#include "VRTrackingVolumeComponent.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "StringUtils.h"
#include "VRObjectSystem.h"

// -- VRTrackingVolumeSystemDefinition -----
VRTrackingVolumeSystemDefinition::VRTrackingVolumeSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

configuru::Config VRTrackingVolumeSystemDefinition::writeToJSON()
{
	configuru::Config pt = Super::writeToJSON();

	return pt;
}

void VRTrackingVolumeSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	Super::readFromJSON(pt);
}

// -- VRTrackingVolumeSystem -----
VRTrackingVolumeSystem::VRTrackingVolumeSystem(ProjectManagerPtr ownerObjectSystemManager)
	: Super::MikanTypedObjectSystem(ownerObjectSystemManager)
{
}

bool VRTrackingVolumeSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	// First call parent init to create all objects
	if (!Super::init(definitionPtr))
		return false;

	// Then ensure VR runtimes are created for all existing volumes
	auto vrObjectSystem = getOwnerProjectManager()->getSystemOfType<VRObjectSystem>();
	for (const auto& pair : Super::getComponentMap())
	{
		VRTrackingVolumeComponentPtr vrVolumeComponent = pair.second.lock();
		if (vrVolumeComponent)
		{
			VRTrackingVolumeDefinitionPtr vrDef = vrVolumeComponent->getVRTrackingVolumeDefinition();
			vrObjectSystem->createTrackingRuntime(vrDef->getTrackingRuntime());
		}
	}

	return true;
}

VRTrackingVolumeIdList VRTrackingVolumeSystem::getTrackingVolumeIdList() const
{
	VRTrackingVolumeIdList trackingVolumeIdList;
	for (const auto& it : Super::getComponentMap())
	{
		trackingVolumeIdList.push_back(it.first);
	}

	return trackingVolumeIdList;
}

VRTrackingVolumeComponentPtr VRTrackingVolumeSystem::addNewVRTrackingVolume(eTrackingRuntime trackingRuntime)
{
	// Ensure the VRObjectSystem has a runtime for this type
	auto vrObjectSystem = getOwnerProjectManager()->getSystemOfType<VRObjectSystem>();
	if (!vrObjectSystem->createTrackingRuntime(trackingRuntime))
	{
		return VRTrackingVolumeComponentPtr();
	}

	// Use the base class method to create the object, with a custom definition init
	return Super::addNewObject([trackingRuntime](VRTrackingVolumeDefinitionPtr def) {
		def->setTrackingRuntime(trackingRuntime);
	});
}

void VRTrackingVolumeSystem::additionalComponentFactory(
	MikanObjectPtr ownerComponentObject,
	VRTrackingVolumeDefinitionPtr componentDefinition)
{
	// Ensure the VRObjectSystem has a runtime for this type
	auto vrObjectSystem = getOwnerProjectManager()->getSystemOfType<VRObjectSystem>();
	vrObjectSystem->createTrackingRuntime(componentDefinition->getTrackingRuntime());
}
