#include "ProfileConfigConstants.h"

const std::string g_patternTypeStrings[(int)eCalibrationPatternType::COUNT] = {
	"chessboard",
	"circlegrid"
};
const std::string* k_patternTypeStrings = g_patternTypeStrings;

const std::string g_stencilTypeStrings[(int)eStencilType::COUNT] = {
	"quad",
	"box",
	"model",
};
extern const std::string* k_stencilTypeStrings = g_stencilTypeStrings;