#include "TestOpenGLMath.h"

glm::mat4 MikanMatrix4f_to_glm_mat4(const MikanMatrix4f& xform)
{
	auto m= reinterpret_cast<const float(*)[4][4]>(&xform);

	glm::mat4 mat= {
		{(*m)[0][0], (*m)[0][1], (*m)[0][2], (*m)[0][3]}, // columns 0
		{(*m)[1][0], (*m)[1][1], (*m)[1][2], (*m)[1][3]}, // columns 1
		{(*m)[2][0], (*m)[2][1], (*m)[2][2], (*m)[2][3]}, // columns 2
		{(*m)[3][0], (*m)[3][1], (*m)[3][2], (*m)[3][3]}, // columns 3
	};

	return mat;
}

glm::vec3 MikanVector3f_to_glm_vec3(const MikanVector3f& in) { return glm::vec3(in.x, in.y, in.z); }

MikanVector3f glm_vec3_to_MikanVector3f(const glm::vec3& in) { return {in.x, in.y, in.z}; }

glm::quat MikanQuatf_to_glm_quat(const MikanQuatf& in) { return glm::quat(in.w, in.x, in.y, in.w); }

glm::mat4 mikan_camera_pose_to_glm_view_matrix(const MikanVector3f& inForward, const MikanVector3f& inUp,
											   const MikanVector3f& inPosition)
{
	const glm::vec3& cameraForward= MikanVector3f_to_glm_vec3(inForward);
	const glm::vec3& cameraUp= MikanVector3f_to_glm_vec3(inUp);
	const glm::vec3& cameraPosition= MikanVector3f_to_glm_vec3(inPosition);

	return glm::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);
}

// fx, fy - focal lengths in pixels
// cx, cy - principal point in pixels
// width, height - image dimensions in pixels
// zNear, zFar - near and far clipping planes
glm::mat4 mikan_camera_intrinsics_to_glm_projection_matrix(float fx, float fy, float cx, float cy, float width,
														   float height, float zNear, float zFar)
{
	// Convert to frustum parameters
	float left= -cx * zNear / fx;
	float right= (width - cx) * zNear / fx;
	float bottom= -(height - cy) * zNear / fy;
	float top= cy * zNear / fy;

	// Create the perspective matrix
	return glm::frustum(left, right, bottom, top, zNear, zFar);
}