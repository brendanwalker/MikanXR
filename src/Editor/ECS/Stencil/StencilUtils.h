#pragma once

#include "ComponentFwd.h"
#include "MikanStencilTypes.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <glm/glm.hpp>

namespace StencilUtils
{
	StencilComponentPtr getStencilById(ProjectManagerPtr projectManager, MikanStencilID stencilId);
	eStencilType getStencilType(ProjectManagerPtr projectManager, MikanStencilID stencilId);
	bool getStencilWorldTransform(ProjectManagerPtr projectManager, MikanStencilID parentStencilId, glm::mat4& outXform);
	bool removeStencil(ProjectManagerPtr projectManager, MikanStencilID stencilId);
}
