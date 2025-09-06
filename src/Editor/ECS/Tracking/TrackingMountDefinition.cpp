#include "TrackingMountDefinition.h"
#include "App.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- TrackingMountDefinition -----
const std::string TrackingMountDefinition::k_devicePathPropertyId = "devicePath";
const std::string TrackingMountDefinition::k_socketNamePropertyId = "socketName";

TrackingMountDefinition::TrackingMountDefinition() 
	: MikanComponentDefinition()
	, m_trackingMountId(INVALID_MIKAN_ID)
{}

TrackingMountDefinition::TrackingMountDefinition(
	MikanTrackingMountID trackingMountId,
	const std::string& markerName)
	: MikanComponentDefinition(markerName)
	, m_trackingMountId(trackingMountId)
{
}

configuru::Config TrackingMountDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();
	
	pt["tracking_mount_id"] = m_trackingMountId;
	pt["device_path"] = m_devicePath;
	pt["socket_name"] = m_socketName;

	return pt;
}

void TrackingMountDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_trackingMountId = pt.get_or<MikanTrackingMountID>("tracking_mount_id", m_trackingMountId);
	m_devicePath = pt.get_or<std::string>("device_path", m_devicePath);
	m_socketName = pt.get_or<std::string>("socket_name", m_socketName);
}

void TrackingMountDefinition::setDevicePath(const std::string& devicePath)
{
	if (devicePath != m_devicePath)
	{
		m_devicePath = devicePath;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_devicePathPropertyId));
	}
}

void TrackingMountDefinition::setSocketName(const std::string& socketName)
{
	if (socketName != m_socketName)
	{
		m_socketName = socketName;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_socketNamePropertyId));
	}
}

// -- IPropertyInterface ----
void TrackingMountDefinition::getPropertyNamesStatic(std::vector<std::string>& outPropertyNames)
{
	outPropertyNames.push_back(MikanComponentDefinition::k_componentNamePropertyId);
	outPropertyNames.push_back(TrackingMountDefinition::k_devicePathPropertyId);
	outPropertyNames.push_back(TrackingMountDefinition::k_socketNamePropertyId);
}

void TrackingMountDefinition::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	TrackingMountDefinition::getPropertyNamesStatic(outPropertyNames);
}

bool TrackingMountDefinition::getPropertyDescriptor(
	const std::string& propertyName,
	PropertyDescriptor& outDescriptor) const
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outDescriptor = { MikanComponentDefinition::k_componentNamePropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name };
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_devicePathPropertyId)
	{
		outDescriptor = { TrackingMountDefinition::k_devicePathPropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name };
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_socketNamePropertyId)
	{
		outDescriptor = { TrackingMountDefinition::k_socketNamePropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name };
		return true;
	}

	return false;
}

bool TrackingMountDefinition::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outValue = getComponentName();
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_devicePathPropertyId)
	{
		outValue = getDevicePath();
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_socketNamePropertyId)
	{
		outValue = getSocketName();
		return true;
	}

	return false;
}

bool TrackingMountDefinition::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{
	return false;
}

bool TrackingMountDefinition::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		setComponentName(inValue.Get<Rml::String>());
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_devicePathPropertyId)
	{
		setDevicePath(inValue.Get<Rml::String>());
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_socketNamePropertyId)
	{
		setSocketName(inValue.Get<Rml::String>());
		return true;
	}

	return false;
}