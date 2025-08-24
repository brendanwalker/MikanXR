#pragma once

#include "TrackingSystemDefinition.h"
#include "TrackingMountDefinition.h"
#include <vector>

class VRTrackingSystemDefinition : public TrackingSystemDefinition
{
public:
	VRTrackingSystemDefinition();
	VRTrackingSystemDefinition(
		eTrackingRuntime trackingRuntime,
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName);

	virtual eTrackingSystemType getTrackingSystemType() const override;

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
