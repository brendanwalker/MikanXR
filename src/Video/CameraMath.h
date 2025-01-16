#pragma once

// -- includes -----
#include "OpenCVFwd.h"
#include "DeviceViewFwd.h"
#include "VideoSourceInterface.h"

#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/matrix_float4x4.hpp"

#include <memory>

#define DEFAULT_MONO_HFOV   60.0   // 60 degrees
#define DEFAULT_MONO_ZNEAR  0.1    // 0.1 meters (10cm)
#define DEFAULT_MONO_ZFAR   20.0   // 20 meters

using t_opengl_point3d_list = std::vector<glm::vec3>;

struct OpenCVCalibrationGeometry
{
	t_opencv_point3d_list points;
};

struct OpenGLCalibrationGeometry
{
	t_opengl_point3d_list points;
};

// -- interface -----
glm::mat4 computeGLMCameraViewMatrix(const glm::mat4& poseXform);
bool computeOpenCVCameraExtrinsicMatrix(VideoSourceViewPtr videoSource, VRDevicePoseViewPtr trackingPuck, cv::Matx34f &out);

bool computeMonoLensCameraCalibration(
	const int frameWidth,
	const int frameHeight,
	const OpenCVCalibrationGeometry& opencvLensCalibrationGeometry,
	const std::vector<t_opencv_point2d_list>& cvImagePointsList,
	const std::vector<t_opencv_pointID_list>& cvImagePointIDs,
	struct MikanMonoIntrinsics& outIntrinsics,
	double& outReprojectionError);

bool computeOpenCVCameraRelativePatternTransform(
	const struct MikanMonoIntrinsics& intrinsics,
	const t_opencv_point2d_list& imagePoints,
	const t_opencv_point3d_list& objectPointsMM,
	cv::Quatd& outOrientation,
	cv::Vec3d& outPositionMM,
	double *outMeanError= nullptr);
void convertOpenCVCameraRelativePoseToGLMMat(
	const cv::Quatd& orientation,
	const cv::Vec3d& positionMM,
	glm::dmat4& outXform);

void extractCameraIntrinsicMatrixParameters(
	const struct MikanMatrix3d& intrinsic_matrix,
	float& out_focal_length_x,
	float& out_focal_length_y,
	float& out_principal_point_x,
	float& out_principal_point_y,
	float& out_skew);
void extractCameraIntrinsicMatrixParameters(
	const cv::Matx33f& intrinsic_matrix,
	float& out_focal_length_x,
	float& out_focal_length_y,
	float& out_principal_point_x,
	float& out_principal_point_y,
	float& out_skew);
bool computeOpenCVCameraRectification(
	VideoSourceViewPtr videoSource,
	VideoFrameSection section,
	cv::Matx33d &rotationOut,
	cv::Matx34d &projectionOut);

void createDefautMonoIntrinsics(
	int pixelWidth,
	int pixelHeight,
	struct MikanMonoIntrinsics& outIntrinsics);

void computeOpenGLProjMatFromCameraIntrinsics(
	const struct MikanMonoIntrinsics& intrinsics,
	glm::mat4& outProjection,
	int* outViewport= nullptr);

enum class eStereoIntrinsicsSide { left, right };
void computeOpenGLProjMatFromCameraIntrinsics(
	const struct MikanStereoIntrinsics& intrinsics,
	eStereoIntrinsicsSide side,
	glm::mat4& outProjection,
	int* outViewport = nullptr);