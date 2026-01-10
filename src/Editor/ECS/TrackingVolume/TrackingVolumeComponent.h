#pragma once

#include "CommonConfig.h"

#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"
#include "TrackingVolumeComponent.h"

#include <memory>
#include <string>

class TrackingVolumeDefinition :
	public MikanComponentDefinition
{
public:
	TrackingVolumeDefinition();
	TrackingVolumeDefinition(
		MikanTrackingVolumeID trackingVolumeId,
		const std::string& trackingVolumeName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MarkerObjectSystemPtr getMarkerObjectSystem() const;

	virtual eTrackingVolumeType getTrackingVolumeType() const { return eTrackingVolumeType::INVALID; }
	inline MikanTrackingVolumeID getTrackingVolumeId() const { return m_trackingVolumeId; }

	static const std::string k_originMarkerIdPropertyId;
	inline MikanMarkerID getOriginMarkerId() const { return m_originMarkeId; }
	MarkerDefinitionConstPtr getOriginMarker() const;
	void setOriginMarkerId(MikanMarkerID arucoId);

private:
	MikanTrackingVolumeID m_trackingVolumeId;
	MikanMarkerID m_originMarkeId;
};

class TrackingVolumeComponent : public MikanComponent
{
public:
	TrackingVolumeComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

	inline static const std::string k_componentClassName = "TrackingVolumeComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline TrackingVolumeDefinitionPtr getTrackingVolumeDefinition() const
	{ return std::static_pointer_cast<TrackingVolumeDefinition>(m_definition); }

	void deleteTrackingVolume();

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(PropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(PropertyDescriptorConstPtr propertyDesc, const MikanVariant& inValue) override;

	// -- IRmlFunctionInterface ----
	static void getRmlFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
	{ MikanComponent::getRmlFunctionDescriptors(outDescriptors); }
};