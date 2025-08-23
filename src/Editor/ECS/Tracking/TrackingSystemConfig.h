#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class TrackingSystemDefinition : public MikanComponentDefinition
{
public:
	TrackingSystemDefinition();
	TrackingSystemDefinition(
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	inline MikanTrackingSystemID getTrackingSystemId() const { return m_trackingSystemId; }

	static const std::string k_originMarkerPropertyId;
	inline MikanMarkerID getOriginMarkerId() const { return m_originMarkeId; }
	void setOriginMarkerId(MikanMarkerID arucoId);

private:
	MikanTrackingSystemID m_trackingSystemId;
	MikanMarkerID m_originMarkeId;
};

class MarkerTrackingSystemDefinition : public TrackingSystemDefinition
{
public:
	MarkerTrackingSystemDefinition() : TrackingSystemDefinition() {}
	MarkerTrackingSystemDefinition(
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName) 
		: TrackingSystemDefinition(trackingSystemId, trackingSystemName) {}
};

class VRTrackingSystemDefinition : public TrackingSystemDefinition
{
public:
	VRTrackingSystemDefinition() : TrackingSystemDefinition() {}
	VRTrackingSystemDefinition(
		eTrackingRuntime trackingRuntime,
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_charucoMountIdPropertyId;
	inline MikanTrackingMountID getCharucoTrackingMountId() const { return m_charucoMountId; }
	void setCharucoTrackingMountId(MikanTrackingMountID mountId);

	static const std::string k_utilityMarkerIdPropertyId;
	inline MikanMarkerID getUtilityMarkerId() const { return m_utilityMarkerId; }
	void setUtilityMarkerId(MikanMarkerID markerId);

	static const std::string k_trackingMountsPropertyId;
	TrackingMountDefinitionConstPtr getTrackingMountDefinitionConst(MikanTrackingMountID mountId) const;
	TrackingMountDefinitionPtr getTrackingMountDefinition(MikanTrackingMountID mountId);
	MikanTrackingMountID addTrackingMount(const std::string& mountName);
	bool removeTrackingMount(MikanTrackingMountID mountId);

private:
	eTrackingRuntime m_trackingRuntime = eTrackingRuntime::INVALID;
	MikanTrackingMountID m_charucoMountId;
	MikanMarkerID m_utilityMarkerId;
	std::vector<TrackingMountDefinitionPtr> m_trackingMounts;
	MikanTrackingMountID m_nextTrackingMountId = 0;
};

class TrackingMountDefinition : public MikanComponentDefinition
{
public:
	TrackingMountDefinition();
	TrackingMountDefinition(
		MikanTrackingMountID trackingMountId,
		const std::string& mountName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	inline MikanTrackingMountID getTrackingMountId() const { return m_trackingMountId; }

	static const std::string k_devicePathPropertyId;
	inline const std::string& getDevicePath() const { return m_devicePath; }
	void setDevicePath(const std::string& devicePath);

	static const std::string k_socketNamePropertyId;
	inline const std::string& getSocketName() const { return m_socketName; }
	void setSocketName(const std::string& socketName);

private:
	MikanTrackingMountID m_trackingMountId;
	std::string m_devicePath;
	std::string m_socketName;
};

class TrackingSystemsConfig : public CommonConfig
{
public:
	TrackingSystemsConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	bool canAddTrackingSystemType(eTrackingSystemType systemType) const;
	eTrackingSystemType getTrackingSystemType(MikanTrackingSystemID systemId) const;
	bool removeTrackingSystem(MikanTrackingSystemID systemId);

	static const std::string k_markerTrackingSystemListPropertyId;
	MarkerTrackingSystemDefinitionConstPtr getMarkerTrackingSystemConfigConst(MikanTrackingSystemID systemId) const;
	MarkerTrackingSystemDefinitionPtr getMarkerTrackingSystemConfig(MikanTrackingSystemID systemId);
	MikanTrackingSystemID addMarkerTrakingSystem(const std::string& trackingSystemName);
	bool removeMarkerTrackingSystem(MikanTrackingSystemID systemId);

	static const std::string k_vrTrackingSystemListPropertyId;
	VRTrackingSystemDefinitionConstPtr getVRTrackingSystemConfigConst(MikanTrackingSystemID systemId) const;
	VRTrackingSystemDefinitionPtr getVRTrackingSystemConfig(MikanTrackingSystemID systemId);
	MikanTrackingSystemID addVRTrackingSystem(eTrackingRuntime trackingRuntime, const std::string& trackingSystemName);
	bool removeVRTrackingSystem(MikanTrackingSystemID systemId);

protected:
	std::vector<MarkerTrackingSystemDefinitionPtr> m_markerTrackingSystemList;
	std::vector<VRTrackingSystemDefinitionPtr> m_vrTrackingSystemList;
	MikanTrackingSystemID m_nextTrackingSystemId = 0;
};