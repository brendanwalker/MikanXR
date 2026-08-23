#include "Constants_PointCloudAlignment.h"

const std::string g_PointCloudAlignmentMenuStateStrings[(int)ePointCloudAlignmentMenuState::COUNT]= {
	"inactive",
	"pendingVideoStart",
	"failedVideoStartStreamRequest",
	"verifyInitialCameraSetup",
	"paintRegionOfInterest",
	"captureFeatureCloud",
	"reviewCloud",
	"runAutoAlignment",
	"verifyAlignment"};
const std::string* k_PointCloudAlignmentMenuStateStrings= g_PointCloudAlignmentMenuStateStrings;
