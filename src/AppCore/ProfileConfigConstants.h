#pragma once

#include <string>

// -- Constants ----
enum class eCalibrationPatternType : int
{
	INVALID = -1,

	mode_chessboard,
	mode_circlegrid,

	COUNT
};
extern const std::string* k_patternTypeStrings;

enum class eStencilType : int
{
	INVALID = -1,

	quad,
	box,
	model,

	COUNT
};
extern const std::string* k_stencilTypeStrings;
