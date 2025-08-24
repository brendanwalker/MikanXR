#include "MarkerDefinition.h"
#include "App.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "StringUtils.h"

// -- MarkerDefinition -----
const std::string MarkerDefinition::k_arucoIdPropertyId= "aruco_id";
const std::string MarkerDefinition::k_lengthMMPropertyId= "length_mm";

MarkerDefinition::MarkerDefinition()
	: MikanComponentDefinition()
{
	m_markerId = INVALID_MIKAN_ID;
}

MarkerDefinition::MarkerDefinition(
	MikanMarkerID markerId,
	const std::string& markerName)
	: MikanComponentDefinition(markerName)
	, m_markerId(markerId)
{}

configuru::Config MarkerDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["id"] = m_markerId;
	pt["aruco_id"] = m_arucoId;
	pt["length_mm"] = m_lengthMM;

	return pt;
}

void MarkerDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_markerId = pt.get<int>("id");
		m_arucoId = pt.get_or<int>("aruco_id", 0);
		m_lengthMM = pt.get_or<float>("length_mm", 100.0f); // Default length is 100mm

		m_configName = StringUtils::stringify("Marker_", m_markerId);
	}
}

void MarkerDefinition::setArucoId(int arucoId)
{
	if (arucoId != m_arucoId)
	{
		m_arucoId = arucoId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_arucoIdPropertyId));
	}
}

void MarkerDefinition::setLengthMM(float lengthMM)
{
	if (lengthMM != m_lengthMM)
	{
		m_lengthMM = lengthMM;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_lengthMMPropertyId));
	}
}
