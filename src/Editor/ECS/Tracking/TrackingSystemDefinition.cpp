#include "TrackingSystemDefinition.h"
#include "App.h"
#include "MarkerSystemConfig.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- TrackingSystemDefinition -----
const std::string TrackingSystemDefinition::k_originMarkerPropertyId = "originMarker";

TrackingSystemDefinition::TrackingSystemDefinition() 
	: MikanComponentDefinition()
	, m_trackingSystemId(INVALID_MIKAN_ID)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}

TrackingSystemDefinition::TrackingSystemDefinition(
	MikanTrackingSystemID trackingSystemId,
	const std::string& trackingSystemName)
	: MikanComponentDefinition(trackingSystemName)
	, m_trackingSystemId(trackingSystemId)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}

configuru::Config TrackingSystemDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();
	
	pt["tracking_system_id"] = m_trackingSystemId;
	pt["origin_marker_id"] = m_originMarkeId;

	return pt;
}

void TrackingSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_trackingSystemId = pt.get_or<MikanTrackingSystemID>("tracking_system_id", m_trackingSystemId);
	m_originMarkeId = pt.get_or<MikanMarkerID>("origin_marker_id", m_originMarkeId);
}

void TrackingSystemDefinition::setOriginMarkerId(MikanMarkerID arucoId)
{
	if (arucoId != m_originMarkeId)
	{
		m_originMarkeId = arucoId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_originMarkerPropertyId));
	}
}

MarkerDefinitionConstPtr TrackingSystemDefinition::getOriginMarker() const
{
	if (m_originMarkeId == INVALID_MIKAN_ID)
	{
		return MarkerSystemConfig::getSystemConfig()->getMarkerConfig(m_originMarkeId);
	}

	return MarkerDefinitionConstPtr();
}

// -- IPropertyInterface ----
void TrackingSystemDefinition::getPropertyNamesStatic(std::vector<std::string>& outPropertyNames)
{
	outPropertyNames.push_back(MikanComponentDefinition::k_componentNamePropertyId);
	outPropertyNames.push_back(TrackingSystemDefinition::k_originMarkerPropertyId);
}

void TrackingSystemDefinition::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	getPropertyNamesStatic(outPropertyNames);
}

bool TrackingSystemDefinition::getPropertyDescriptor(
	const std::string& propertyName, 
	PropertyDescriptor& outDescriptor) const
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outDescriptor = { MikanComponentDefinition::k_componentNamePropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name };
		return true;
	}
	else if (propertyName == TrackingSystemDefinition::k_originMarkerPropertyId)
	{
		outDescriptor = { TrackingSystemDefinition::k_originMarkerPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::marker_id };
		return true;
	}

	return false;
}

bool TrackingSystemDefinition::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outValue = getComponentName();
		return true;
	}
	else if (propertyName == TrackingSystemDefinition::k_originMarkerPropertyId)
	{
		outValue = getOriginMarkerId();
		return true;
	}

	return false;
}

bool TrackingSystemDefinition::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{	
	return false;
}

bool TrackingSystemDefinition::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		setComponentName(inValue.Get<Rml::String>());
		return true;
	}
	else if (propertyName == TrackingSystemDefinition::k_originMarkerPropertyId)
	{
		if (inValue.GetType() == Rml::Variant::INT)
		{
			MikanMarkerID markerId = inValue.Get<int>();
			setOriginMarkerId(markerId);
			return true;
		}
	}

	return false;
}