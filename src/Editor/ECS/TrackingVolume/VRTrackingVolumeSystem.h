#pragma once

#include "ComponentFwd.h"
#include "VRTrackingVolumeComponent.h"
#include "MikanTypedObjectSystem.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <memory>
#include <string>
#include <vector>

using VRTrackingVolumeIdList= std::vector<MikanTrackingVolumeID>;

class VRTrackingVolumeSystemDefinition
	: public MikanTypedObjectSystemDefinition<VRTrackingVolumeComponent, VRTrackingVolumeDefinition,
											  MikanTrackingVolumeID>
{
public:
	using Super=
		MikanTypedObjectSystemDefinition<VRTrackingVolumeComponent, VRTrackingVolumeDefinition, MikanTrackingVolumeID>;

	VRTrackingVolumeSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
};

class VRTrackingVolumeSystem
	: public MikanTypedObjectSystem<VRTrackingVolumeComponent, VRTrackingVolumeDefinition, MikanTrackingVolumeID,
									VRTrackingVolumeSystem, VRTrackingVolumeSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<VRTrackingVolumeComponent, VRTrackingVolumeDefinition, MikanTrackingVolumeID,
										VRTrackingVolumeSystem, VRTrackingVolumeSystemDefinition>;

	VRTrackingVolumeSystem(ProjectManagerPtr ownerObjectSystemManager);

	inline static const std::string k_objectSystemClassName= "VRTrackingVolumeSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;

	VRTrackingVolumeIdList getTrackingVolumeIdList() const;

	inline VRTrackingVolumeComponentPtr getVRTrackingVolumeById(MikanTrackingVolumeID trackingVolumeId) const
	{
		return Super::getTypedComponentById(trackingVolumeId);
	}

	VRTrackingVolumeComponentPtr addNewVRTrackingVolume(eTrackingRuntime trackingRuntime);

protected:
	virtual void additionalComponentFactory(MikanObjectPtr ownerComponentObject,
											VRTrackingVolumeDefinitionPtr componentDefinition) override;
};
