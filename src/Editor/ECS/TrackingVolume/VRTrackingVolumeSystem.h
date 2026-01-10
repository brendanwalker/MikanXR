#pragma once

#include "ComponentFwd.h"
#include "VRTrackingVolumeComponent.h"
#include "MikanObjectSystem.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

using VRTrackingVolumeMap = std::map<MikanTrackingVolumeID, VRTrackingVolumeComponentWeakPtr>;
using VRTrackingVolumeIdList = std::vector<MikanTrackingVolumeID>;

class VRTrackingVolumeSystemDefinition : public MikanObjectSystemDefinition
{
public:
	VRTrackingVolumeSystemDefinition(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_vrTrackingVolumeListPropertyId;
	const std::vector<VRTrackingVolumeDefinitionPtr>& getVRTrackingVolumeList() const { return m_vrTrackingVolumeList; }
	VRTrackingVolumeDefinitionConstPtr getVRTrackingVolumeDefinitionConst(MikanTrackingVolumeID trackingVolumeId) const;
	VRTrackingVolumeDefinitionPtr getVRTrackingVolumeDefinition(MikanTrackingVolumeID trackingVolumeId);
	MikanTrackingVolumeID addVRTrackingVolume(eTrackingRuntime trackingRuntime);
	bool removeVRTrackingVolume(MikanTrackingVolumeID trackingVolumeId);

protected:
	std::vector<VRTrackingVolumeDefinitionPtr> m_vrTrackingVolumeList;
	MikanTrackingVolumeID m_nextTrackingVolumeId = 0;
};

class VRTrackingVolumeSystem : public MikanObjectSystem
{
public:
	VRTrackingVolumeSystem(ProjectManagerPtr ownerObjectSystemManager) : MikanObjectSystem(ownerObjectSystemManager) {}

	inline static const std::string k_objectSystemClassName = "VRTrackingVolumeSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;

	VRTrackingVolumeSystemDefinitionConstPtr getVRTrackingVolumeSystemConfigConst() const;
	VRTrackingVolumeSystemDefinitionPtr getVRTrackingVolumeSystemConfig();

	virtual MikanComponentPtr getComponentById(int componentId) const override;

	const VRTrackingVolumeMap& getVRTrackingVolumeMap() const { return m_vrTrackingVolumeComponents; }
	VRTrackingVolumeIdList getTrackingVolumeIdList() const;
	VRTrackingVolumeComponentPtr getVRTrackingVolumeById(MikanTrackingVolumeID trackingVolumeId) const;

	VRTrackingVolumeComponentPtr addNewVRTrackingVolume(eTrackingRuntime trackingRuntime);
	bool removeVRTrackingVolume(MikanTrackingVolumeID trackingVolumeId);

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	VRTrackingVolumeComponentPtr createVRTrackingVolumeObject(VRTrackingVolumeDefinitionPtr trackingVolumeConfig);
	void disposeVRTrackingVolumeObject(MikanTrackingVolumeID trackingVolumeId);

	VRTrackingVolumeMap m_vrTrackingVolumeComponents;
};
