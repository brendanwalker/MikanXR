#pragma once

#include "ComponentFwd.h"
#include <glm/gtc/matrix_transform.hpp>

bool align_stencil_fastener_to_anchor_fastener(
	FastenerComponentPtr sourceFastener,
	FastenerComponentPtr targetFastener,
	glm::mat4& outNewStencilXform,
	glm::vec3 outNewStencilPoints[3]);