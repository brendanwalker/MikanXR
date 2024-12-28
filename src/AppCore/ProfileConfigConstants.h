#pragma once

#include <string>

// -- Constants ----
enum class eCalibrationPatternType : int
{
	INVALID = -1,

	mode_chessboard,
	mode_charuco,
	mode_aruco,

	COUNT
};
extern const std::string* k_patternTypeStrings;

enum class eCharucoDictionaryType : int
{
	INVALID = -1,

	DICT_4X4,
	DICT_5X5,
	DICT_6X6,
	DICT_7X7,

	COUNT
};
extern const std::string* k_charucoDictionaryStrings;

enum class eStencilType : int
{
	INVALID = -1,

	quad,
	box,
	model,

	COUNT
};
extern const char** k_szStencilTypeStrings;
extern const std::string* k_stencilTypeStrings;
