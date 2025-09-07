#include "TrackingAPIDefinition.h"
#include "App.h"
#include "MarkerObjectSystem.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// TODO: Replace App singleton access
#include "MainWindow.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- TrackingAPIDefinition -----
const std::string TrackingAPIDefinition::k_originMarkerPropertyId = "originMarker";

TrackingAPIDefinition::TrackingAPIDefinition() 
	: MikanComponentDefinition()
	, m_trackingSystemId(INVALID_MIKAN_ID)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}

TrackingAPIDefinition::TrackingAPIDefinition(
	MikanTrackingSystemID trackingSystemId,
	const std::string& trackingSystemName)
	: MikanComponentDefinition(trackingSystemName)
	, m_trackingSystemId(trackingSystemId)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}

configuru::Config TrackingAPIDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();
	
	pt["tracking_system_id"] = m_trackingSystemId;
	pt["origin_marker_id"] = m_originMarkeId;

	return pt;
}

void TrackingAPIDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_trackingSystemId = pt.get_or<MikanTrackingSystemID>("tracking_system_id", m_trackingSystemId);
	m_originMarkeId = pt.get_or<MikanMarkerID>("origin_marker_id", m_originMarkeId);
}

MarkerObjectSystemPtr TrackingAPIDefinition::getMarkerObjectSystem() const
{
	// TODO: Replace App singleton access
	return App::getInstance()->getMainWindow()->getObjectSystemManager()->getSystemOfType<MarkerObjectSystem>();
}

void TrackingAPIDefinition::markDirty(const ConfigPropertyChangeSet& changedPropertySet)
{
	CommonConfig::markDirty(changedPropertySet);

	if (OnPropertyChanged)
	{
		// TODO: Only notify for property names that are actually exposed in getPropertyNames()
		for (const std::string& changedPropertyName : changedPropertySet.getSet())
		{
			OnPropertyChanged(changedPropertyName);
		}
	}
}

void TrackingAPIDefinition::setOriginMarkerId(MikanMarkerID arucoId)
{
	if (arucoId != m_originMarkeId)
	{
		m_originMarkeId = arucoId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_originMarkerPropertyId));
	}
}

MarkerDefinitionConstPtr TrackingAPIDefinition::getOriginMarker() const
{
	if (m_originMarkeId == INVALID_MIKAN_ID)
	{
		return getMarkerObjectSystem()->getMarkerSystemConfig()->getMarkerConfig(m_originMarkeId);
	}

	return MarkerDefinitionConstPtr();
}

// -- IPropertyInterface ----
void TrackingAPIDefinition::getPropertyNamesStatic(std::vector<std::string>& outPropertyNames)
{
	outPropertyNames.push_back(MikanComponentDefinition::k_componentNamePropertyId);
	outPropertyNames.push_back(TrackingAPIDefinition::k_originMarkerPropertyId);
}

void TrackingAPIDefinition::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	getPropertyNamesStatic(outPropertyNames);
}

bool TrackingAPIDefinition::getPropertyDescriptor(
	const std::string& propertyName, 
	PropertyDescriptor& outDescriptor) const
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outDescriptor = { MikanComponentDefinition::k_componentNamePropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name };
		return true;
	}
	else if (propertyName == TrackingAPIDefinition::k_originMarkerPropertyId)
	{
		outDescriptor = { TrackingAPIDefinition::k_originMarkerPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::marker_id };
		return true;
	}

	return false;
}

bool TrackingAPIDefinition::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outValue = getComponentName();
		return true;
	}
	else if (propertyName == TrackingAPIDefinition::k_originMarkerPropertyId)
	{
		outValue = getOriginMarkerId();
		return true;
	}

	return false;
}

bool TrackingAPIDefinition::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{	
	return false;
}

bool TrackingAPIDefinition::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		setComponentName(inValue.Get<Rml::String>());
		return true;
	}
	else if (propertyName == TrackingAPIDefinition::k_originMarkerPropertyId)
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