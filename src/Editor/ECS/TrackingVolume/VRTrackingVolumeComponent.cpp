#include "VRTrackingVolumeComponent.h"
#include "App.h"
#include "MarkerObjectSystem.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- VRTrackingVolumeDefinition -----
const std::string VRTrackingVolumeDefinition::k_charucoMountIdPropertyId = "charucoMountId";
const std::string VRTrackingVolumeDefinition::k_charucoMountOffsetPropertyId = "charucoMountOffsetMM";
const std::string VRTrackingVolumeDefinition::k_utilityMarkerIdPropertyId = "utilityMarkerId";
const std::string VRTrackingVolumeDefinition::k_trackingMountIdsPropertyId = "trackingMountIds";

VRTrackingVolumeDefinition::VRTrackingVolumeDefinition()
	: TrackingVolumeDefinition()
{
}

VRTrackingVolumeDefinition::VRTrackingVolumeDefinition(
	eTrackingRuntime trackingRuntime,
	MikanTrackingVolumeID trackingVolumeId,
	const std::string& trackingVolumeName)
	: TrackingVolumeDefinition(trackingVolumeId, trackingVolumeName)
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

eTrackingVolumeType VRTrackingVolumeDefinition::getTrackingVolumeType() const
{
	return eTrackingVolumeType::vr;
}

configuru::Config VRTrackingVolumeDefinition::writeToJSON()
{
	configuru::Config pt = TrackingVolumeDefinition::writeToJSON();

	pt["tracking_runtime"] = k_trackingRuntimeStrings[(int)m_trackingRuntime];
	pt["charuco_mount_id"] = m_charucoMountId;
	pt["utility_marker_id"] = m_utilityMarkerId;

	writeVector3f(pt, "charuco_mount_offset_mm", m_charucoMountOffsetMM);
	writeStdValueVector(pt, "tracking_mount_ids", m_trackingMountIDs);

	return pt;
}

void VRTrackingVolumeDefinition::readFromJSON(const configuru::Config& pt)
{
	TrackingVolumeDefinition::readFromJSON(pt);

	const std::string trackingRuntimeString =
		pt.get_or<std::string>("tracking_runtime", k_trackingRuntimeStrings[0]);
	m_trackingRuntime =
		StringUtils::FindEnumValue<eTrackingRuntime>(trackingRuntimeString, k_trackingRuntimeStrings);
	m_charucoMountId = pt.get_or<MikanTrackingMountID>("charuco_mount_id", INVALID_MIKAN_ID);
	m_utilityMarkerId = pt.get_or<MikanMarkerID>("utility_marker_id", INVALID_MIKAN_ID);

	readVector3f(pt, "charuco_mount_offset_mm", m_charucoMountOffsetMM);
	readStdValueVector(pt, "tracking_mount_ids", m_trackingMountIDs);
}

void VRTrackingVolumeDefinition::setCharucoTrackingMountId(MikanTrackingMountID mountId)
{
	if (mountId != m_charucoMountId)
	{
		m_charucoMountId = mountId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoMountIdPropertyId));
	}
}

void VRTrackingVolumeDefinition::setCharucoMountOffsetMM(const MikanVector3f& offset)
{
	m_charucoMountOffsetMM = offset;
	markDirty(ConfigPropertyChangeSet().addPropertyName("charucoMountOffsetMM"));
}

void VRTrackingVolumeDefinition::setUtilityMarkerId(MikanMarkerID markerId)
{
	if (markerId != m_utilityMarkerId)
	{
		m_utilityMarkerId = markerId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_utilityMarkerIdPropertyId));
	}
}

bool VRTrackingVolumeDefinition::addTrackingMountID(MikanTrackingMountID mountId)
{
	auto it = std::find(m_trackingMountIDs.begin(), m_trackingMountIDs.end(), mountId);
	if (it == m_trackingMountIDs.end())
	{
		m_trackingMountIDs.push_back(mountId);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingMountIdsPropertyId));
		return true;
	}

	return false;
}

bool VRTrackingVolumeDefinition::removeTrackingMountID(MikanTrackingMountID mountId)
{
	auto it = std::find(m_trackingMountIDs.begin(), m_trackingMountIDs.end(), mountId);
	if (it != m_trackingMountIDs.end())
	{
		m_trackingMountIDs.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingMountIdsPropertyId));
		return true;
	}

	return false;
}

// -- VRTrackingVolumeComponent -----
VRTrackingVolumeComponent::VRTrackingVolumeComponent(MikanObjectWeakPtr owner)
	: TrackingVolumeComponent(owner)
{
}

// -- IRmlPropertyInterface ----
void VRTrackingVolumeComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	TrackingVolumeComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			VRTrackingVolumeDefinition::k_charucoMountIdPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			VRTrackingVolumeDefinition::k_charucoMountOffsetPropertyId)
		->setDefaultValue(Rml::Vector3f(0.f)));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			VRTrackingVolumeDefinition::k_utilityMarkerIdPropertyId));
}

bool VRTrackingVolumeComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc, 
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == VRTrackingVolumeDefinition::k_charucoMountIdPropertyId)
	{
		outValue = getVRTrackingVolumeDefinition()->getCharucoTrackingMountId();
		return true;
	}
	else if (propertyName == VRTrackingVolumeDefinition::k_charucoMountOffsetPropertyId)
	{
		const MikanVector3f& offset = getVRTrackingVolumeDefinition()->getCharucoMountOffsetMM();
		outValue = Rml::Vector3f(offset.x, offset.y, offset.z);
		return true;
	}
	else if (propertyName == VRTrackingVolumeDefinition::k_utilityMarkerIdPropertyId)
	{
		outValue = getVRTrackingVolumeDefinition()->getUtilityMarkerId();
		return true;
	}

	return TrackingVolumeComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool VRTrackingVolumeComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == VRTrackingVolumeDefinition::k_charucoMountIdPropertyId)
	{
		getVRTrackingVolumeDefinition()->setCharucoTrackingMountId(inValue.Get<int>());
		return true;
	}
	else if (propertyName == VRTrackingVolumeDefinition::k_charucoMountOffsetPropertyId)
	{
		Rml::Vector3 offset = inValue.Get<Rml::Vector3f>();

		getVRTrackingVolumeDefinition()->setCharucoMountOffsetMM({ offset.x, offset.y, offset.z });
		return true;
	}
	else if (propertyName == VRTrackingVolumeDefinition::k_utilityMarkerIdPropertyId)
	{
		getVRTrackingVolumeDefinition()->setUtilityMarkerId(inValue.Get<int>());
		return true;
	}

	return TrackingVolumeComponent::setPropertyValueFromRml(propertyDesc, inValue);
}