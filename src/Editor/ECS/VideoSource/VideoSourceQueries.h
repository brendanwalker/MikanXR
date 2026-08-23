#pragma once

#include "ComponentFwd.h"
#include "MikanVideoSourceTypes.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <vector>

using VideoSourceIdList= std::vector<MikanVideoSourceID>;

namespace VideoSourceQueries
{
VideoSourceIdList getVideoSourceIdList(ProjectManagerConstPtr projectManager);
VideoSourceComponentPtr getVideoSourceById(ProjectManagerConstPtr projectManager, MikanVideoSourceID videoSourceId);
eVideoSourceType getVideoSourceType(ProjectManagerConstPtr projectManager, MikanVideoSourceID videoSourceId);
bool removeVideoSource(ProjectManagerConstPtr projectManager, MikanVideoSourceID videoSourceId);
} // namespace VideoSourceQueries
