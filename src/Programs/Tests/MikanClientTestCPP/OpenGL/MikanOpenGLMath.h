#include "MikanMathTypes.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

glm::mat4 MikanMatrix4f_to_glm_mat4(const MikanMatrix4f& xform);
glm::vec3 MikanVector3f_to_glm_vec3(const MikanVector3f& in);
MikanVector3f glm_vec3_to_MikanVector3f(const glm::vec3& in);

glm::mat4 mikan_camera_pose_to_glm_view_matrix(
	const MikanVector3f& cameraForward,
	const MikanVector3f& cameraUp,
	const MikanVector3f& cameraPosition);

// fx, fy - focal lengths in pixels
// cx, cy - principal point in pixels
// width, height - image dimensions in pixels
// zNear, zFar - near and far clipping planes
glm::mat4 mikan_camera_intrinsics_to_glm_projection_matrix(
	float fx, float fy,
	float cx, float cy,
	float width, float height,
	float zNear, float zFar);
