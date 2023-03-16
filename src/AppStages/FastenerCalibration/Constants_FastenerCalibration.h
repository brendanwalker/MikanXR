#pragma once

#include <string>

enum class eFastenerCalibrationMenuState : int
{
	INVALID = -1,

	inactive,
	verifyInitialCameraSetup,
	captureInitialPoints,
	verifyInitialPointCapture,
	moveCamera,
	captureTriangulatedPoints,
	verifyTriangulatedPoints,
	testCalibration,
	failedVideoStartStreamRequest,

	COUNT
};
extern const std::string* k_fastenerCalibrationMenuStateStrings;

enum class eFastenerCalibrationViewpointMode : int
{
	INVALID = -1,

	mixedRealityViewpoint,
	vrViewpoint,

	COUNT
};
extern const std::string* k_fastenerCalibrationViewpointModeStrings;