#pragma once

#include "nlohmann/json.hpp"

#include "MikanVideoSourceTypes.h"
#include "MikanMathTypes_json.h"

NLOHMANN_JSON_SERIALIZE_ENUM(MikanVideoSourceType, {
	{MikanVideoSourceType_MONO, "MONO"},
	{MikanVideoSourceType_STEREO, "STEREO"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(MikanVideoSourceApi, {
	{MikanVideoSourceApi_INVALID, nullptr},
	{MikanVideoSourceApi_OPENCV_CV, "OPEN_CV"},
	{MikanVideoSourceApi_WINDOWS_MEDIA_FOUNDATION, "WMF"},
})


NLOHMANN_JSON_SERIALIZE_ENUM(MikanIntrinsicsType, {
	{INVALID_CAMERA_INTRINSICS, nullptr},
	{MONO_CAMERA_INTRINSICS, "MONO_CAMERA_INTRINSICS"},
	{STEREO_CAMERA_INTRINSICS, "STEREO_CAMERA_INTRINSICS"},
})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanDistortionCoefficients, k1, k2, k3, k4, k5, k6, p1, p2)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanMonoIntrinsics, pixel_width, pixel_height, hfov, vfov, znear, zfar, distortion_coefficients, camera_matrix)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanStereoIntrinsics, pixel_width, pixel_height, hfov, vfov, znear, zfar, left_distortion_coefficients, left_camera_matrix, right_distortion_coefficients, right_camera_matrix, left_rectification_rotation, right_rectification_rotation, left_rectification_projection, right_rectification_projection, rotation_between_cameras, translation_between_cameras, essential_matrix, fundamental_matrix, reprojection_matrix)

inline void to_json(nlohmann::json& j, const MikanVideoSourceIntrinsics& p)
{
	if (p.intrinsics_type == MONO_CAMERA_INTRINSICS)
		j = nlohmann::json{{"mono", p.intrinsics.mono}, {"intrinsics_type", p.intrinsics_type}};
	else if (p.intrinsics_type == STEREO_CAMERA_INTRINSICS)
		j = nlohmann::json{{"stereo", p.intrinsics.stereo}, {"intrinsics_type", p.intrinsics_type}};
}
inline void from_json(const nlohmann::json& j, MikanVideoSourceIntrinsics& p)
{
	j.at("intrinsics_type").get_to(p.intrinsics_type);
	if (p.intrinsics_type == MONO_CAMERA_INTRINSICS)
		j.at("mono").get_to(p.intrinsics.mono);
	else if (p.intrinsics_type == STEREO_CAMERA_INTRINSICS)
		j.at("stereo").get_to(p.intrinsics.stereo);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVideoSourceAttachmentInfo, attached_vr_device_id, vr_device_offset_xform)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVideoSourceMode, video_source_type, video_source_api, device_path, video_mode_name, resolution_x, resolution_y, frame_rate)
