#pragma once

// -- includes -----
#include "OpenCVFwd.h"
#include "VideoSourceInterface.h"
#include "MikanMathTypes.h"

#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/matrix_float4x4.hpp"

#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

// -- interface -----
glm::mat4 computeGLMCameraViewMatrix(const glm::mat4& poseXform);
void computeOpenCVCameraExtrinsicMatrix(VideoSourceViewPtr videoSource, VRDeviceViewPtr trackingPuck, cv::Matx34f &out);

bool computeOpenCVCameraRelativePatternTransform(
	const MikanMonoIntrinsics& intrinsics,
	const t_opencv_point2d_list& imagePoints,
	const t_opencv_point3d_list& objectPointsMM,
	cv::Quatd& outOrientation,
	cv::Vec3d& outPositionMM);
void convertOpenCVCameraRelativePoseToGLMMat(
	const cv::Quatd& orientation,
	const cv::Vec3d& positionMM,
	glm::dmat4& outXform);

void computeOpenCVCameraIntrinsicMatrix(
	VideoSourceViewPtr videoSource,
	VideoFrameSection section,
	cv::Matx33f &intrinsicOut);
void extractCameraIntrinsicMatrixParameters(
	const MikanMatrix3d& intrinsic_matrix,
	float& out_focal_length_x,
	float& out_focal_length_y,
	float& out_principal_point_x,
	float& out_principal_point_y);
void extractCameraIntrinsicMatrixParameters(
	const cv::Matx33f &intrinsic_matrix,
	float &out_focal_length_x,
	float &out_focal_length_y,
	float &out_principal_point_x,
	float &out_principal_point_y);
bool computeOpenCVCameraRectification(
	VideoSourceViewPtr videoSource,
	VideoFrameSection section,
	cv::Matx33d &rotationOut,
	cv::Matx34d &projectionOut);

void createDefautMonoIntrinsics(
	int pixelWidth,
	int pixelHeight,
	MikanMonoIntrinsics& outIntrinsics);