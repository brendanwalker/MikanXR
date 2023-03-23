#pragma once

#include <glm/gtc/matrix_transform.hpp>

typedef int32_t MikanSpatialFastenerID;

bool align_stencil_fastener_to_anchor_fastener(
	MikanSpatialFastenerID sourceId,
	MikanSpatialFastenerID targetId,
	glm::mat4& outNewStencilXform,
	glm::vec3 outNewStencilPoints[3]);