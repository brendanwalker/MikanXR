#pragma once

#include "glm/ext/vector_float4.hpp"

void glStateSetViewport(GlState& glState, int x, int y, int width, int height);
void glStateSetClearColor(GlState& glState, const glm::vec4& color);
