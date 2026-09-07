#include "TestVulkanMath.h"

#include <glm/ext/matrix_clip_space.hpp>

glm::mat4 mikan_camera_intrinsics_to_vulkan_projection_matrix(float fx, float fy, float cx, float cy, float width,
															  float height, float zNear, float zFar)
{
	// The same frustum the OpenGL path derives from the intrinsics
	const float left= -cx * zNear / fx;
	const float right= (width - cx) * zNear / fx;
	const float bottom= -(height - cy) * zNear / fy;
	const float top= cy * zNear / fy;

	// Zero-to-one depth for Vulkan, then flip y since Vulkan's NDC y points down
	glm::mat4 projection= glm::frustumRH_ZO(left, right, bottom, top, zNear, zFar);
	glm::mat4 flipY(1.0f);
	flipY[1][1]= -1.0f;

	return flipY * projection;
}
