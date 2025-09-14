#include "TrackingVolumeComponent.h"
#include "App.h"
#include "MathTypeConversion.h"
#include "MarkerObjectSystem.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"
#include "TrackingVolumeObjectSystem.h"

// TODO: Replace App singleton access
#include "MainWindow.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- TrackingVolumeDefinition -----
const std::string TrackingVolumeDefinition::k_originMarkerPropertyId = "originMarker";

TrackingVolumeDefinition::TrackingVolumeDefinition()
	: MikanComponentDefinition()
	, m_trackingVolumeId(INVALID_MIKAN_ID)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}

TrackingVolumeDefinition::TrackingVolumeDefinition(
	MikanTrackingVolumeID trackingVolumeId,
	const std::string& trackingVolumeName)
	: MikanComponentDefinition(trackingVolumeName)
	, m_trackingVolumeId(trackingVolumeId)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}

configuru::Config TrackingVolumeDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["tracking_volume_id"] = m_trackingVolumeId;
	pt["origin_marker_id"] = m_originMarkeId;

	return pt;
}

void TrackingVolumeDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_trackingVolumeId = pt.get_or<MikanTrackingVolumeID>("tracking_volume_id", m_trackingVolumeId);
	m_originMarkeId = pt.get_or<MikanMarkerID>("origin_marker_id", m_originMarkeId);
}

MarkerObjectSystemPtr TrackingVolumeDefinition::getMarkerObjectSystem() const
{
	// TODO: Replace App singleton access
	return App::getInstance()->getMainWindow()->getObjectSystemManager()->getSystemOfType<MarkerObjectSystem>();
}

void TrackingVolumeDefinition::setOriginMarkerId(MikanMarkerID arucoId)
{
	if (arucoId != m_originMarkeId)
	{
		m_originMarkeId = arucoId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_originMarkerPropertyId));
	}
}

MarkerDefinitionConstPtr TrackingVolumeDefinition::getOriginMarker() const
{
	if (m_originMarkeId == INVALID_MIKAN_ID)
	{
		return getMarkerObjectSystem()->getMarkerSystemConfig()->getMarkerConfig(m_originMarkeId);
	}

	return MarkerDefinitionConstPtr();
}

// -- TrackingVolumeComponent -----
TrackingVolumeComponent::TrackingVolumeComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

void TrackingVolumeComponent::init()
{
	MikanComponent::init();
}

TrackingVolumeObjectSystemPtr TrackingVolumeComponent::getOwnerTrackingVolumeSystem() const
{
	return getObjectSystemOfType<TrackingVolumeObjectSystem>();
}

void TrackingVolumeComponent::deleteTrackingVolume()
{
	TrackingVolumeDefinitionPtr def = getTrackingVolumeDefinition();
	if (def)
	{
		TrackingVolumeObjectSystemPtr system = getOwnerTrackingVolumeSystem();
		if (system)
		{
			system->removeTrackingVolume(def->getTrackingVolumeId());
		}
	}
}

// -- IRmlPropertyInterface ----
void TrackingVolumeComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			TrackingVolumeDefinition::k_originMarkerPropertyId));
}

bool TrackingVolumeComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyId = propertyDesc->getName();

	if (propertyId == TrackingVolumeDefinition::k_originMarkerPropertyId)
	{
		outValue= getTrackingVolumeDefinition()->getTrackingVolumeId();
		return true;
	}

	return MikanComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool TrackingVolumeComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyId = propertyDesc->getName();

	if (propertyId == TrackingVolumeDefinition::k_originMarkerPropertyId)
	{
		MikanMarkerID markerId = inValue.Get<int>();
		getTrackingVolumeDefinition()->setOriginMarkerId(markerId);
		return true;
	}

	return MikanComponent::setPropertyValueFromRml(propertyDesc, inValue);
}