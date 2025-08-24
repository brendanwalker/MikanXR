#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class TrackingSystemDefinition : public MikanComponentDefinition
{
public:
	TrackingSystemDefinition();
	TrackingSystemDefinition(
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	virtual eTrackingSystemType getTrackingSystemType() const { return eTrackingSystemType::INVALID; }
	inline MikanTrackingSystemID getTrackingSystemId() const { return m_trackingSystemId; }

	static const std::string k_originMarkerPropertyId;
	inline MikanMarkerID getOriginMarkerId() const { return m_originMarkeId; }
	MarkerDefinitionConstPtr getOriginMarker() const;
	void setOriginMarkerId(MikanMarkerID arucoId);

private:
	MikanTrackingSystemID m_trackingSystemId;
	MikanMarkerID m_originMarkeId;
};
