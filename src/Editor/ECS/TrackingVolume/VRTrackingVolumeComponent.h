#pragma once

#include "MikanMathTypes.h"
#include "TrackingMountComponent.h"
#include "TrackingVolumeComponent.h"
#include "VRTrackingVolumeComponent.h"

#include <vector>

class VRTrackingVolumeDefinition : public TrackingVolumeDefinition
{
public:
	VRTrackingVolumeDefinition();
	VRTrackingVolumeDefinition(
		eTrackingRuntime trackingRuntime,
		MikanTrackingVolumeID trackingVolumeId,
		const std::string& trackingVolumeName);

	virtual eTrackingVolumeType getTrackingVolumeType() const override;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_charucoMountIdPropertyId;
	inline MikanTrackingMountID getCharucoTrackingMountId() const { return m_charucoMountId; }
	void setCharucoTrackingMountId(MikanTrackingMountID mountId);

	static const std::string k_charucoMountOffsetPropertyId;
	inline MikanVector3f getCharucoMountOffsetMM() const { return m_charucoMountOffsetMM; }
	void setCharucoMountOffsetMM(const MikanVector3f& offset);

	static const std::string k_utilityMarkerIdPropertyId;
	inline MikanMarkerID getUtilityMarkerId() const { return m_utilityMarkerId; }
	void setUtilityMarkerId(MikanMarkerID markerId);

	static const std::string k_trackingMountIdsPropertyId;
	inline const std::vector<MikanTrackingMountID>& getTrackingMountIDs() const { return m_trackingMountIDs; }
	bool addTrackingMountID(MikanTrackingMountID mountId);
	bool removeTrackingMountID(MikanTrackingMountID mountId);

private:
	eTrackingRuntime m_trackingRuntime = eTrackingRuntime::INVALID;
	MikanTrackingMountID m_charucoMountId;
	MikanVector3f m_charucoMountOffsetMM;
	MikanMarkerID m_utilityMarkerId;
	std::vector<MikanTrackingMountID> m_trackingMountIDs;
};

class VRTrackingVolumeComponent : public TrackingVolumeComponent
{
public:
	VRTrackingVolumeComponent(MikanObjectWeakPtr owner);

	inline VRTrackingVolumeDefinitionPtr getVRTrackingVolumeDefinition() const
	{ return std::static_pointer_cast<VRTrackingVolumeDefinition>(m_definition); }

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	static void getFunctionNamesStatic(std::vector<std::string>& outPropertyNames)
	{ TrackingVolumeComponent::getFunctionNamesStatic(outPropertyNames); }
};