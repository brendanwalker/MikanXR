#include "Constants_AlignCameraByOriginMarker.h"

const std::string g_alignCameraByOriginMarkerMenuStateStrings[(int)eAlignCameraByOriginMarkerMenuState::COUNT]= {
	"inactive", "pendingVideoStart", "verifySetup", "capturing", "testCalibration", "failedVideoStart"};
const std::string* k_alignCameraByOriginMarkerMenuStateStrings= g_alignCameraByOriginMarkerMenuStateStrings;
