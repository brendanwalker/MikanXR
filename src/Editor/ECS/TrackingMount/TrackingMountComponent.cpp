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
		getOwnerTrackingMountSystem()->removeTrackingMountID(trackingMountId);
	}
}

// -- IRmlPropertyInterface ----
void TrackingMountComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			TrackingMountDefinition::k_devicePathPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			TrackingMountDefinition::k_socketNamePropertyId));
}

bool TrackingMountComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == TrackingMountDefinition::k_devicePathPropertyId)
	{
		outValue = getTrackingMountDefinition()->getDevicePath();
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_socketNamePropertyId)
	{
		outValue = getTrackingMountDefinition()->getSocketName();
		return true;
	}

	return MikanComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool TrackingMountComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == TrackingMountDefinition::k_devicePathPropertyId)
	{
		getTrackingMountDefinition()->setDevicePath(inValue.Get<Rml::String>());
		return true;
	}
	else if (propertyName == TrackingMountDefinition::k_socketNamePropertyId)
	{
		getTrackingMountDefinition()->setSocketName(inValue.Get<Rml::String>());
		return true;
	}

	return MikanComponent::setPropertyValueFromRml(propertyDesc, inValue);
}