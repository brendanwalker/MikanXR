#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "PropertyInterface.h"
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

class TrackingVolumeDefinition : 
	public MikanComponentDefinition,
	public IPropertyInterface
{
public:
	TrackingVolumeDefinition();
	TrackingVolumeDefinition(
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
	virtual void markDirty(const ConfigPropertyChangeSet& changedPropertySet) override;

	MarkerObjectSystemPtr getMarkerObjectSystem() const;

	virtual eTrackingSystemType getTrackingSystemType() const { return eTrackingSystemType::INVALID; }
	inline MikanTrackingSystemID getTrackingSystemId() const { return m_trackingSystemId; }

	static const std::string k_originMarkerPropertyId;
	inline MikanMarkerID getOriginMarkerId() const { return m_originMarkeId; }
	MarkerDefinitionConstPtr getOriginMarker() const;
	void setOriginMarkerId(MikanMarkerID arucoId);

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

private:
	MikanTrackingSystemID m_trackingSystemId;
	MikanMarkerID m_originMarkeId;
};