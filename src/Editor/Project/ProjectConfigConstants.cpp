#include "ProjectConfigConstants.h"

const std::string g_patternTypeStrings[(int)eCalibrationPatternType::COUNT] = {
	"chessboard",
	"charuco",
	"aruco"
};
const std::string* k_patternTypeStrings = g_patternTypeStrings;

const std::string g_charucoDictionaryStrings[(int)eCharucoDictionaryType::COUNT] = {
	"DICT_4X4",
	"DICT_5X5",
	"DICT_6X6",
	"DICT_7X7"
};
const std::string* k_charucoDictionaryStrings = g_charucoDictionaryStrings;

const char* g_szStencilTypeStrings[(int)eStencilType::COUNT] = {
	"quad",
	"box",
	"model",
};
const std::string g_stencilTypeStrings[(int)eStencilType::COUNT] = {
	g_szStencilTypeStrings[0],
	g_szStencilTypeStrings[1],
	g_szStencilTypeStrings[2],
};
const char** k_szStencilTypeStrings= g_szStencilTypeStrings;
const std::string* k_stencilTypeStrings = g_stencilTypeStrings;

const char* g_szTrackingSystemTypeStrings[(int)eTrackingSystemType::COUNT] = {
	"marker",
	"vr",
};
const std::string g_TrackingSystemTypeStrings[(int)eTrackingSystemType::COUNT] = {
	g_szTrackingSystemTypeStrings[0],
	g_szTrackingSystemTypeStrings[1],
};
const char** k_szTrackingSystemTypeStrings= g_szTrackingSystemTypeStrings;
const std::string* k_trackingSystemTypeStrings= g_TrackingSystemTypeStrings;

const std::string g_trackingRuntimeStrings[(int)eTrackingRuntime::COUNT] = {
	"SteamVR"
};
const std::string* k_trackingRuntimeStrings = g_trackingRuntimeStrings;

const std::string g_videoSourceTypeStrings[(int)eVideoSourceType::COUNT] = {
	"usb",
	"networked",
	"spout"
};
extern const char** k_szVideoSourceTypeStrings;
extern const std::string* k_videoSourceTypeStrings;