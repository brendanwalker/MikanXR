#include "ProfileConfigConstants.h"
#include "MikanClientTypes.h"

const std::string g_patternTypeStrings[(int)eCalibrationPatternType::COUNT] = {
	"chessboard",
	"circlegrid"
};
const std::string* k_patternTypeStrings = g_patternTypeStrings;


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