#pragma once

#include "ComponentFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"
#include "TrackingVolumeComponent.h"

#include <vector>

using TrackingVolumeIdList= std::vector<MikanTrackingVolumeID>;

namespace TrackingVolumeQueries
{
TrackingVolumeIdList getTrackingVolumeIdList(ProjectManagerConstPtr projectManager);
TrackingVolumeComponentPtr getTrackingVolumeById(ProjectManagerConstPtr projectManager, MikanTrackingVolumeID trackingVolumeId);
eTrackingVolumeType getTrackingVolumeType(ProjectManagerConstPtr projectManager, MikanTrackingVolumeID trackingVolumeId);
bool removeTrackingVolume(ProjectManagerConstPtr projectManager, MikanTrackingVolumeID trackingVolumeId);
} // namespace TrackingVolumeQueries
