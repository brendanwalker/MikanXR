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
