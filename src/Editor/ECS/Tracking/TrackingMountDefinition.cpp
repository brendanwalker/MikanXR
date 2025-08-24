#include "TrackingMountDefinition.h"
#include "App.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

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
