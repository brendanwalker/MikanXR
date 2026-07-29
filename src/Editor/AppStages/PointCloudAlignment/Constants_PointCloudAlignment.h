#pragma once

#include <string>

enum class ePointCloudAlignmentMenuState : int
{
	INVALID= -1,

	inactive,
	pendingVideoStart,
	failedVideoStartStreamRequest,
	verifyInitialCameraSetup,
	paintRegionOfInterest,
	captureFeatureCloud,
	reviewCloud,
	runAutoAlignment,
	verifyAlignment,

	COUNT
};
extern const std::string* k_PointCloudAlignmentMenuStateStrings;
