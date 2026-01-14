#include "MarkerTrackingVolumeSystem.h"
#include "Logger.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "StringUtils.h"

// -- MarkerTrackingVolumeSystemDefinition -----
MarkerTrackingVolumeSystemDefinition::MarkerTrackingVolumeSystemDefinition(const std::string& configName)
	: Super::MikanTypedObjectSystemDefinition(configName)
{
}

configuru::Config MarkerTrackingVolumeSystemDefinition::writeToJSON()
{
	configuru::Config pt = Super::writeToJSON();

	return pt;
}

void MarkerTrackingVolumeSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	Super::readFromJSON(pt);
}

// -- MarkerTrackingVolumeSystem -----
MarkerTrackingVolumeSystem::MarkerTrackingVolumeSystem(ProjectManagerPtr ownerObjectSystemManager)
	: Super::MikanTypedObjectSystem(ownerObjectSystemManager)
{
}

MarkerTrackingVolumeIdList MarkerTrackingVolumeSystem::getTrackingVolumeIdList() const
{
	MarkerTrackingVolumeIdList trackingVolumeIdList;
	for (const auto& it : Super::getComponentMap())
	{
		trackingVolumeIdList.push_back(it.first);
	}

	return trackingVolumeIdList;
}
