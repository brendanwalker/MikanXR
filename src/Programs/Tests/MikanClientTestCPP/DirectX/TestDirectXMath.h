#include "MikanMathTypes.h"

#include <d3d11_1.h>
#include <directxmath.h>

DirectX::XMFLOAT3 mikan_vec3_to_directx_xmfloat3(const MikanVector3f& vec);
DirectX::XMVECTOR mikan_vec3_to_directx_xmvector(const MikanVector3f& vec);
DirectX::XMVECTOR mikan_position_to_directx_xmvector(const MikanVector3f& vec);

DirectX::XMMATRIX mikan_camera_pose_to_directx_view_matrix(const MikanVector3f& cameraForward,
														   const MikanVector3f& cameraUp,
														   const MikanVector3f& cameraPosition);

// fx, fy - focal lengths in pixels
// cx, cy - principal point in pixels
// width, height - image dimensions in pixels
// zNear, zFar - near and far clipping planes
DirectX::XMMATRIX mikan_camera_intrinsics_to_directx_projection_matrix(float fx, float fy, float cx, float cy,
																	   float width, float height, float zNear,
																	   float zFar);
