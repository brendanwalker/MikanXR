#include "VRTrackingSystemDefinition.h"
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

// -- VRTrackingSystemDefinition -----
const std::string VRTrackingSystemDefinition::k_charucoMountIdPropertyId= "charucoMountId";
const std::string VRTrackingSystemDefinition::k_charucoMountOffsetPropertyId = "charucoMountOffsetMM";
const std::string VRTrackingSystemDefinition::k_utilityMarkerIdPropertyId= "utilityMarkerId";
const std::string VRTrackingSystemDefinition::k_trackingMountsPropertyId= "trackingMounts";

VRTrackingSystemDefinition::VRTrackingSystemDefinition()
	: TrackingSystemDefinition()
{
}

VRTrackingSystemDefinition::VRTrackingSystemDefinition(
	eTrackingRuntime trackingRuntime,
	MikanTrackingSystemID trackingSystemId,
	const std::string& trackingSystemName)
	: TrackingSystemDefinition(trackingSystemId, trackingSystemName)
	, m_trackingRuntime(trackingRuntime)
	, m_charucoMountId(INVALID_MIKAN_ID)
	, m_utilityMarkerId(INVALID_MIKAN_ID)
	, m_charucoMountOffsetMM({
		DEFAULT_PUCK_HORIZONTAL_OFFSET_MM,
		DEFAULT_PUCK_VERTICAL_OFFSET_MM,
		DEFAULT_PUCK_DEPTH_OFFSET_MM
		})
{
}

eTrackingSystemType VRTrackingSystemDefinition::getTrackingSystemType() const
{
	return eTrackingSystemType::vr;
}

configuru::Config VRTrackingSystemDefinition::writeToJSON()
{
	configuru::Config pt = TrackingSystemDefinition::writeToJSON();

	pt["tracking_runtime"] = k_trackingRuntimeStrings[(int)m_trackingRuntime];
	pt["charuco_mount_id"] = m_charucoMountId;
	pt["utility_marker_id"] = m_utilityMarkerId;
	pt["next_tracking_mount_id"] = m_nextTrackingMountId;

	writeVector3f(pt, "charuco_mount_offset_mm", m_charucoMountOffsetMM);

	std::vector<configuru::Config> trackingMountConfigs;
	for (auto trackingMount : m_trackingMounts)
	{
		trackingMountConfigs.push_back(trackingMount->writeToJSON());
	}
	pt.insert_or_assign(std::string("tracking_mounts"), trackingMountConfigs);

	return pt;
}

void VRTrackingSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	TrackingSystemDefinition::readFromJSON(pt);

	const std::string trackingRuntimeString = 
		pt.get_or<std::string>("tracking_runtime", k_trackingRuntimeStrings[0]);
	m_trackingRuntime = 
		StringUtils::FindEnumValue<eTrackingRuntime>(trackingRuntimeString, k_trackingRuntimeStrings);
	m_charucoMountId = pt.get_or<MikanTrackingMountID>("charuco_mount_id", INVALID_MIKAN_ID);
	m_utilityMarkerId = pt.get_or<MikanMarkerID>("utility_marker_id", INVALID_MIKAN_ID);
	
	readVector3f(pt, "charuco_mount_offset_mm", m_charucoMountOffsetMM);

	if (pt.has_key("tracking_mounts"))
	{
		for (const configuru::Config& trackingMount_pt : pt["tracking_mounts"].as_array())
		{
			auto definitionPtr = std::make_shared<TrackingMountDefinition>();

			definitionPtr->readFromJSON(trackingMount_pt);
			m_trackingMounts.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

void VRTrackingSystemDefinition::setCharucoTrackingMountId(MikanTrackingMountID mountId)
{
	if (mountId != m_charucoMountId)
	{
		m_charucoMountId = mountId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoMountIdPropertyId));
	}
}

TrackingMountDefinitionConstPtr VRTrackingSystemDefinition::getCharucoTrackingMount() const
{
	return getTrackingMountDefinitionConst(m_charucoMountId);
}

void VRTrackingSystemDefinition::setCharucoMountOffsetMM(const MikanVector3f& offset)
{
	m_charucoMountOffsetMM = offset;
	markDirty(ConfigPropertyChangeSet().addPropertyName("charucoMountOffsetMM"));
}

void VRTrackingSystemDefinition::setUtilityMarkerId(MikanMarkerID markerId)
{
	if (markerId != m_utilityMarkerId)
	{
		m_utilityMarkerId = markerId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_utilityMarkerIdPropertyId));
	}
}

TrackingMountDefinitionConstPtr VRTrackingSystemDefinition::getTrackingMountDefinitionConst(
	MikanTrackingMountID mountId) const
{
	auto it=
		std::find_if(
			m_trackingMounts.begin(), m_trackingMounts.end(),
			[mountId](const TrackingMountDefinitionPtr& mountDef) {
				return mountDef->getTrackingMountId() == mountId;
			});

	if (it != m_trackingMounts.end())
	{
		return *it;
	}

	return TrackingMountDefinitionConstPtr();
}

TrackingMountDefinitionPtr VRTrackingSystemDefinition::getTrackingMountDefinition(
	MikanTrackingMountID mountId)
{
	return std::const_pointer_cast<TrackingMountDefinition>(getTrackingMountDefinitionConst(mountId));
}

MikanTrackingMountID VRTrackingSystemDefinition::addTrackingMount()
{
	const std::string mountName = StringUtils::stringify("Mount ", m_nextTrackingMountId);
	TrackingMountDefinitionPtr trackingMountDefinition =
		std::make_shared<TrackingMountDefinition>(m_nextTrackingMountId, mountName);
	m_nextTrackingMountId++;

	m_trackingMounts.push_back(trackingMountDefinition);
	addChildConfig(trackingMountDefinition);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingMountsPropertyId));

	return trackingMountDefinition->getTrackingMountId();
}

bool VRTrackingSystemDefinition::removeTrackingMount(MikanTrackingMountID mountId)
{
	auto it = std::find_if(
		m_trackingMounts.begin(), m_trackingMounts.end(),
		[mountId](const TrackingMountDefinitionPtr& mountDef) {
			return mountDef->getTrackingMountId() == mountId;
		});

	if (it != m_trackingMounts.end())
	{
		removeChildConfig(*it);
		m_trackingMounts.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingMountsPropertyId));

		return true;
	}

	return false;
}

MarkerDefinitionConstPtr VRTrackingSystemDefinition::getUtilityMarker() const
{
	if (m_utilityMarkerId == INVALID_MIKAN_ID)
	{
		return MarkerSystemConfig::getSystemConfig()->getMarkerConfig(m_utilityMarkerId);
	}

	return MarkerDefinitionConstPtr();
}

// -- IPropertyInterface ----
void VRTrackingSystemDefinition::getPropertyNamesStatic(std::vector<std::string>& outPropertyNames)
{
	TrackingSystemDefinition::getPropertyNamesStatic(outPropertyNames);

	outPropertyNames.push_back(VRTrackingSystemDefinition::k_charucoMountIdPropertyId);
	outPropertyNames.push_back(VRTrackingSystemDefinition::k_charucoMountOffsetPropertyId);
	outPropertyNames.push_back(VRTrackingSystemDefinition::k_utilityMarkerIdPropertyId);
}

void VRTrackingSystemDefinition::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	VRTrackingSystemDefinition::getPropertyNamesStatic(outPropertyNames);
}

bool VRTrackingSystemDefinition::getPropertyDescriptor(
	const std::string& propertyName,
	PropertyDescriptor& outDescriptor) const
{
	if (propertyName == VRTrackingSystemDefinition::k_charucoMountIdPropertyId)
	{
		outDescriptor = { VRTrackingSystemDefinition::k_charucoMountIdPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::tracking_mount_id };
		return true;
	}
	else if (propertyName == VRTrackingSystemDefinition::k_charucoMountOffsetPropertyId)
	{
		outDescriptor = { VRTrackingSystemDefinition::k_charucoMountOffsetPropertyId, ePropertyDataType::datatype_float3, ePropertySemantic::position };
		return true;
	}
	else if (propertyName == VRTrackingSystemDefinition::k_utilityMarkerIdPropertyId)
	{
		outDescriptor = { VRTrackingSystemDefinition::k_utilityMarkerIdPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::marker_id };
		return true;
	}

	return TrackingSystemDefinition::getPropertyDescriptor(propertyName, outDescriptor);
}

bool VRTrackingSystemDefinition::getPropertyValue(
	const std::string& propertyName, 
	Rml::Variant& outValue) const
{
	if (propertyName == VRTrackingSystemDefinition::k_charucoMountIdPropertyId)
	{
		outValue = getCharucoTrackingMountId();
		return true;
	}
	else if (propertyName == VRTrackingSystemDefinition::k_charucoMountOffsetPropertyId)
	{
		const MikanVector3f& offset = getCharucoMountOffsetMM();
		outValue = Rml::Vector3f(offset.x, offset.y, offset.z);
		return true;
	}
	else if (propertyName == VRTrackingSystemDefinition::k_utilityMarkerIdPropertyId)
	{
		outValue = getUtilityMarkerId();
		return true;
	}

	return TrackingSystemDefinition::getPropertyValue(propertyName, outValue);
}

bool VRTrackingSystemDefinition::setPropertyValue(
	const std::string& propertyName, 
	const Rml::Variant& inValue)
{
	if (propertyName == VRTrackingSystemDefinition::k_charucoMountIdPropertyId)
	{
		setCharucoTrackingMountId(inValue.Get<int>());
		return true;
	}
	else if (propertyName == VRTrackingSystemDefinition::k_charucoMountOffsetPropertyId)
	{
		Rml::Vector3 offset = inValue.Get<Rml::Vector3f>();

		setCharucoMountOffsetMM({ offset.x, offset.y, offset.z });
		return true;
	}
	else if (propertyName == VRTrackingSystemDefinition::k_utilityMarkerIdPropertyId)
	{
		setUtilityMarkerId(inValue.Get<int>());
		return true;
	}

	return TrackingSystemDefinition::setPropertyValue(propertyName, inValue);
}