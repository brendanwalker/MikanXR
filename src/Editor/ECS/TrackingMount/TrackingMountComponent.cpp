#include "TrackingMountComponent.h"
#include "App.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"
#include "TrackingMountObjectSystem.h"
#include "VRObjectSystem.h"

// -- TrackingMountDefinition -----
const std::string TrackingMountDefinition::k_desiredDevicePathPropertyId = "device_path";
const std::string TrackingMountDefinition::k_socketNamePropertyId = "socket_name";

TrackingMountDefinition::TrackingMountDefinition() 
	: MikanComponentDefinition()
{}

TrackingMountDefinition::TrackingMountDefinition(
	MikanTrackingMountID trackingMountId)
	: MikanComponentDefinition(trackingMountId, "")
{
}

configuru::Config TrackingMountDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();
	
	pt[k_desiredDevicePathPropertyId] = m_devicePath;
	pt[k_socketNamePropertyId] = m_socketName;

	return pt;
}

void TrackingMountDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_devicePath = pt.get_or<std::string>(k_desiredDevicePathPropertyId, m_devicePath);
	m_socketName = pt.get_or<std::string>(k_socketNamePropertyId, m_socketName);
}

void TrackingMountDefinition::setDevicePath(const std::string& devicePath)
{
	if (devicePath != m_devicePath)
	{
		m_devicePath = devicePath;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_desiredDevicePathPropertyId));
	}
}

void TrackingMountDefinition::setSocketName(const std::string& socketName)
{
	if (socketName != m_socketName)
	{
		m_socketName = socketName;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_socketNamePropertyId));
	}
}

// -- TrackingMountComponent -----
TrackingMountComponent::TrackingMountComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{

}

void TrackingMountComponent::init()
{
	MikanComponent::init();
}

TrackingMountObjectSystemPtr TrackingMountComponent::getOwnerTrackingMountSystem() const
{
	return std::static_pointer_cast<TrackingMountObjectSystem>(getOwnerObject()->getOwnerSystem());
}

VRDeviceComponentPtr TrackingMountComponent::getVRDeviceComponent() const
{
	const std::string& vrDevicePath= getTrackingMountDefinition()->getDevicePath();
	auto vrObjectSystem= getObjectSystemOfType<VRObjectSystem>();

	return vrObjectSystem->getVRDeviceByPath(vrDevicePath);
}

void TrackingMountComponent::deleteTrackingMount()
{
	TrackingMountDefinitionPtr trackingMountDefinition = getTrackingMountDefinition();
	if (trackingMountDefinition)
	{
		const MikanTrackingMountID trackingMountId = trackingMountDefinition->getTrackingMountId();
		getOwnerTrackingMountSystem()->removeObject(trackingMountId);
	}
}

// -- IPropertyInterface ----
void TrackingMountComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			TrackingMountDefinition::k_desiredDevicePathPropertyId, MikanVariantType::STRING));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			TrackingMountDefinition::k_socketNamePropertyId, MikanVariantType::STRING));
}

bool TrackingMountComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == TrackingMountDefinition::k_desiredDevicePathPropertyId)
	{
		outValue = getTrackingMountDefinition()->getDevicePath();
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_socketNamePropertyId)
	{
		outValue = getTrackingMountDefinition()->getSocketName();
		return true;
	}

	return MikanComponent::getPropertyValue(propertyName, outValue);
}

bool TrackingMountComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == TrackingMountDefinition::k_desiredDevicePathPropertyId)
	{
		getTrackingMountDefinition()->setDevicePath(inValue.getStringValue());
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_socketNamePropertyId)
	{
		getTrackingMountDefinition()->setSocketName(inValue.getStringValue());
		return true;
	}

	return MikanComponent::setPropertyValue(propertyName, inValue);
}