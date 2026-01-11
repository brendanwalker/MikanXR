#include "TrackingVolumeComponent.h"
#include "App.h"
#include "MathTypeConversion.h"
#include "MarkerObjectSystem.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "SelectionComponent.h"
#include "StringUtils.h"
#include "TrackingVolumeQueries.h"

// -- TrackingVolumeDefinition -----
const std::string TrackingVolumeDefinition::k_originMarkerIdPropertyId = "origin_marker_id";

TrackingVolumeDefinition::TrackingVolumeDefinition()
	: MikanComponentDefinition()
	, m_trackingVolumeId(INVALID_MIKAN_ID)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}

TrackingVolumeDefinition::TrackingVolumeDefinition(
	MikanTrackingVolumeID trackingVolumeId,
	const std::string& trackingVolumeName)
	: MikanComponentDefinition(trackingVolumeId, trackingVolumeName)
	, m_trackingVolumeId(trackingVolumeId)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}

configuru::Config TrackingVolumeDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["tracking_volume_id"] = m_trackingVolumeId;
	pt[k_originMarkerIdPropertyId.c_str()] = m_originMarkeId;

	return pt;
}

void TrackingVolumeDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_trackingVolumeId = pt.get_or<MikanTrackingVolumeID>("tracking_volume_id", m_trackingVolumeId);
	m_originMarkeId = pt.get_or<MikanMarkerID>(k_originMarkerIdPropertyId.c_str(), m_originMarkeId);
}

MarkerObjectSystemPtr TrackingVolumeDefinition::getMarkerObjectSystem() const
{
	return getOwnerComponent()->getObjectSystemOfType<MarkerObjectSystem>();
}

void TrackingVolumeDefinition::setOriginMarkerId(MikanMarkerID arucoId)
{
	if (arucoId != m_originMarkeId)
	{
		m_originMarkeId = arucoId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_originMarkerIdPropertyId));
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

void TrackingVolumeComponent::deleteTrackingVolume()
{
	TrackingVolumeDefinitionPtr def = getTrackingVolumeDefinition();
	if (def)
	{
		ProjectManagerPtr projectManager = getOwnerProjectManager();
		TrackingVolumeQueries::removeTrackingVolume(projectManager, def->getTrackingVolumeId());
	}
}

// -- IPropertyInterface ----
void TrackingVolumeComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			TrackingVolumeDefinition::k_originMarkerIdPropertyId, MikanVariantType::INT)
			->setDefaultValue(-1));
}

bool TrackingVolumeComponent::getPropertyValue(
	PropertyDescriptorConstPtr propertyDesc,
	MikanVariant& outValue) const
{
	const std::string& propertyId = propertyDesc->getName();

	if (propertyId == TrackingVolumeDefinition::k_originMarkerIdPropertyId)
	{
		outValue= getTrackingVolumeDefinition()->getTrackingVolumeId();
		return true;
	}

	return MikanComponent::getPropertyValue(propertyDesc, outValue);
}

bool TrackingVolumeComponent::setPropertyValue(
	PropertyDescriptorConstPtr propertyDesc,
	const MikanVariant& inValue)
{
	const std::string& propertyId = propertyDesc->getName();

	if (propertyId == TrackingVolumeDefinition::k_originMarkerIdPropertyId)
	{
		MikanMarkerID markerId = inValue.getIntValue();
		getTrackingVolumeDefinition()->setOriginMarkerId(markerId);
		return true;
	}

	return MikanComponent::setPropertyValue(propertyDesc, inValue);
}