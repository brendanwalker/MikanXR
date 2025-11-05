#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "TrackingMountComponent.h"
#include "MikanObjectSystem.h"
#include "MikanTypeFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>

using TrackingMountMap = std::map<MikanTrackingMountID, TrackingMountComponentWeakPtr>;

class TrackingMountObjectSystemConfig : public CommonConfig
{
public:
	TrackingMountObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	// TrackingMount Settings
	static const std::string k_trackingMountListPropertyId;
	TrackingMountDefinitionPtr getTrackingMountConfig(MikanTrackingMountID trackingMountId) const;
	TrackingMountDefinitionPtr getTrackingMountConfigByName(const std::string& trackingMountName) const;
	MikanTrackingMountID addNewTrackingMount();
	bool removeTrackingMountID(MikanTrackingMountID trackingMountId);
	const std::vector<TrackingMountDefinitionPtr>& getTrackingMountList() const { return m_trackingMountList; }

private:
	MikanTrackingMountID m_nextTrackingMountId = 0;

	// List of TrackingMount definitions
	std::vector<TrackingMountDefinitionPtr> m_trackingMountList;
};

class TrackingMountObjectSystem : public MikanObjectSystem
{
public:
	TrackingMountObjectSystem(class ProjectManager* ownerObjectSystemManager) : MikanObjectSystem(ownerObjectSystemManager) {}

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	TrackingMountObjectSystemConfigConstPtr getTrackingMountSystemConfigConst() const;
	TrackingMountObjectSystemConfigPtr getTrackingMountSystemConfig();

	const TrackingMountMap& getTrackingMountMap() const { return m_trackingMountComponents; }
	TrackingMountComponentPtr getTrackingMountById(MikanTrackingMountID trackingMountId) const;
	TrackingMountComponentPtr getTrackingMountByName(const std::string& trackingMountName) const;
	TrackingMountComponentPtr addNewTrackingMount();
	bool removeTrackingMountID(MikanTrackingMountID trackingMountId);

protected:
	TrackingMountComponentPtr createTrackingMountObject(TrackingMountDefinitionPtr trackingMountConfig);
	void disposeTrackingMountObject(MikanTrackingMountID trackingMountId);

	TrackingMountMap m_trackingMountComponents;
};