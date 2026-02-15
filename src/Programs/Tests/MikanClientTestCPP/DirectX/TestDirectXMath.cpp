#include "TestDirectXMath.h"

DirectX::XMFLOAT3 mikan_vec3_to_directx_xmfloat3(const MikanVector3f& vec)
{
	return {vec.x, vec.y, -vec.z};
}

DirectX::XMVECTOR mikan_vec3_to_directx_xmvector(const MikanVector3f& vec)
{
	return {vec.x, vec.y, -vec.z, 0.f};
}

DirectX::XMVECTOR mikan_position_to_directx_xmvector(const MikanVector3f& vec)
{
	return {vec.x, vec.y, -vec.z, 1.f};
}

DirectX::XMMATRIX mikan_camera_pose_to_directx_view_matrix(
	const MikanVector3f& cameraForward,
	const MikanVector3f & cameraUp,
	const MikanVector3f& cameraPosition)
{
	// Create the view matrix using DirectX's left-handed function
	const DirectX::XMVECTOR dxForward = mikan_vec3_to_directx_xmvector(cameraForward);
	const DirectX::XMVECTOR dxUp = mikan_vec3_to_directx_xmvector(cameraUp);
	const DirectX::XMVECTOR dxPosition = mikan_position_to_directx_xmvector(cameraPosition);
	const DirectX::XMVECTOR dxTarget = DirectX::XMVectorAdd(dxPosition, dxForward);

	return DirectX::XMMatrixLookAtLH(dxPosition, dxTarget, dxUp);
}

DirectX::XMMATRIX mikan_camera_intrinsics_to_directx_projection_matrix(
	float fx, float fy,
	float cx, float cy,
	float width, float height,
	float zNear, float zFar)
{
	// First, convert focal lengths to normalized device coordinates
	float fovX = 2.0f * atan(width / (2.0f * fx));
	float fovY = 2.0f * atan(height / (2.0f * fy));

	// Convert principal point offset from center (normalized)
	float s_x = 2.0f * (cx / width) - 1.0f;
	float s_y = 1.0f - 2.0f * (cy / height); // Note: Y is flipped in DirectX

	// Build the projection matrix
	float aspectRatio = width / height;
	float tanHalfFovY = tan(fovY / 2.0f);
	float tanHalfFovX = tan(fovX / 2.0f);

	// Left-handed projection matrix (DirectX default)
	// + X is right
	// + Y is up
	// + Z is forward(into the screen)
	DirectX::XMMATRIX projMatrix; 
	projMatrix.r[0] = { 1.0f / (aspectRatio * tanHalfFovY), 0.0f, 0.0f, 0.0f };
	projMatrix.r[1] = { 0.0f, 1.0f / tanHalfFovY, 0.0f, 0.0f };
	projMatrix.r[2] = { s_x, s_y, zFar / (zFar - zNear), 1.0f };
	projMatrix.r[3] = { 0.0f, 0.0f, -zNear * zFar / (zFar - zNear), 0.0f };

	return projMatrix;
}