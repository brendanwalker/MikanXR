#pragma once

#include "VRDeviceMath.h"
#include <glm/glm.hpp>

namespace vr
{
	struct HmdMatrix34_t;
};

// SteamVR types to GLM types
glm::mat4 vr_HmdMatrix34_to_glm_mat4(const vr::HmdMatrix34_t& in);

// GLM types to Mikan VR Interface Types
VRDevicePose glm_mat4_to_VRDevicePose(const glm::mat4& in);