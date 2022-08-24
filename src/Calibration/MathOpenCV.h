#pragma once

#include "OpenCVFwd.h"
#include <opencv2/core/quaternion.hpp>
#include <vector>

// Compute the average of an array of vectors 
bool opencv_vec3d_compute_average(
	const std::vector<cv::Vec3d>& vectors,
	cv::Vec3d& outAverageVec);

// Compute the average of an array of quaternions 
bool opencv_quaternion_compute_average(
	const std::vector<cv::Quatd>& quaternions,
	cv::Quatd& outAverageQuat);