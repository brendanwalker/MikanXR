#include "MikanSteamVRMath.h"
#include "VRDeviceMath.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "openvr.h"

glm::mat4 vr_HmdMatrix34_to_glm_mat4(const vr::HmdMatrix34_t& in)
{
	float mat44[16]= {in.m[0][0], in.m[1][0], in.m[2][0], 0.0f, in.m[0][1], in.m[1][1], in.m[2][1], 0.0f,
					  in.m[0][2], in.m[1][2], in.m[2][2], 0.0f, in.m[0][3], in.m[1][3], in.m[2][3], 1.0f};

	return glm::make_mat4(mat44);
}

VRDevicePose glm_mat4_to_VRDevicePose(const glm::mat4& in)
{
	// https://github.com/ValveSoftware/openvr/wiki/Matrix-Usage-Example
	//  "HmdMatrix34_t is used to represent a rigid transform (rotation plus translation)"

	// Position is in meters
	VRDevicePose pose;
	pose.position.x= in[3][0];
	pose.position.y= in[3][1];
	pose.position.z= in[3][2];

	// Convert 3x3 rotation matrix to quaternion
	glm::quat orientation= glm::quat_cast(in);
	pose.orientation.w= orientation.w;
	pose.orientation.x= orientation.x;
	pose.orientation.y= orientation.y;
	pose.orientation.z= orientation.z;

	return pose;
}