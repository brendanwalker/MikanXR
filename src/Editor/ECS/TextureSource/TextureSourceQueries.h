#pragma once

#include "ComponentFwd.h"
#include "MikanTextureSourceTypes.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <vector>

using TextureSourceIdList= std::vector<MikanTextureSourceID>;
using TextureSourceComponentList= std::vector<TextureSourceComponentPtr>;

namespace TextureSourceQueries
{
TextureSourceComponentList getTextureSourceComponentList(ProjectManagerPtr projectManager);
TextureSourceIdList getTextureSourceIdList(ProjectManagerPtr projectManager);
TextureSourceComponentPtr getTextureSourceById(ProjectManagerPtr projectManager, MikanTextureSourceID textureSourceId);
eTextureSourceType getTextureSourceType(ProjectManagerPtr projectManager, MikanTextureSourceID textureSourceId);
bool removeTextureSource(ProjectManagerPtr projectManager, MikanTextureSourceID textureSourceId);
} // namespace TextureSourceQueries
