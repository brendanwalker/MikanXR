#include "ProfileConfigConstants.h"

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
extern const char** k_szStencilTypeStrings= g_szStencilTypeStrings;
const std::string* k_stencilTypeStrings = g_stencilTypeStrings;