#pragma once

#include "MikanMathTypes.h"
#include "TrackingAPIDefinition.h"
#include "TrackingMountDefinition.h"
#include <vector>

class VRTrackingAPIDefinition : 
	public TrackingAPIDefinition
{
public:
	VRTrackingAPIDefinition();
	VRTrackingAPIDefinition(
		eTrackingRuntime trackingRuntime,
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName);

	virtual eTrackingSystemType getTrackingSystemType() const override;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_charucoMountIdPropertyId;
	inline MikanTrackingMountID getCharucoTrackingMountId() const { return m_charucoMountId; }
	void setCharucoTrackingMountId(MikanTrackingMountID mountId);
	TrackingMountDefinitionConstPtr getCharucoTrackingMount() const;

	static const std::string k_charucoMountOffsetPropertyId;
	inline MikanVector3f getCharucoMountOffsetMM() const { return m_charucoMountOffsetMM; }
	void setCharucoMountOffsetMM(const MikanVector3f& offset);

	static const std::string k_utilityMarkerIdPropertyId;
	inline MikanMarkerID getUtilityMarkerId() const { return m_utilityMarkerId; }
	void setUtilityMarkerId(MikanMarkerID markerId);
	MarkerDefinitionConstPtr getUtilityMarker() const;

	static const std::string k_trackingMountsPropertyId;
	inline const std::vector<TrackingMountDefinitionPtr>& getTrackingMounts() const { return m_trackingMounts; }
	TrackingMountDefinitionConstPtr getTrackingMountDefinitionConst(MikanTrackingMountID mountId) const;
	TrackingMountDefinitionPtr getTrackingMountDefinition(MikanTrackingMountID mountId);
	MikanTrackingMountID addTrackingMount();
	bool removeTrackingMount(MikanTrackingMountID mountId);

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

private:
	eTrackingRuntime m_trackingRuntime = eTrackingRuntime::INVALID;
	MikanTrackingMountID m_charucoMountId;
	MikanVector3f m_charucoMountOffsetMM;

	MikanMarkerID m_utilityMarkerId;
	std::vector<TrackingMountDefinitionPtr> m_trackingMounts;
	MikanTrackingMountID m_nextTrackingMountId = 0;
};