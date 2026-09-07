#pragma once

#include "MikanMathTypes.h"

#include <glm/glm.hpp>

// fx, fy - focal lengths in pixels
// cx, cy - principal point in pixels
// width, height - image dimensions in pixels
// zNear, zFar - near and far clipping planes
// Right-handed like the OpenGL projection, with Vulkan's clip conventions: depth in 0..1 and NDC y down.
glm::mat4 mikan_camera_intrinsics_to_vulkan_projection_matrix(float fx, float fy, float cx, float cy, float width,
															  float height, float zNear, float zFar);
