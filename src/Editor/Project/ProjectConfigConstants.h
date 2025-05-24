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

#define CHESSBOARD_PATTERN_W				7 // Internal corners
#define CHESSBOARD_PATTERN_H				5
#define DEFAULT_SQUARE_LEN_MM				30

#define CHARUCO_PATTERN_W					11
#define CHARUCO_PATTERN_H					8
#define DEFAULT_CHARUCO_SQUARE_LEN_MM		16
#define DEFAULT_CHARUCO_MARKER_LEN_MM		12
#define DEFAULT_CHARUCO_DICTIONARY_TYPE		eCharucoDictionaryType::DICT_6X6

#define DEFAULT_PUCK_HORIZONTAL_OFFSET_MM	211 // 51mm puck to pattern + 16mm squares *10squares
#define DEFAULT_PUCK_VERTICAL_OFFSET_MM		48 // 16mm square * 3 squares
#define DEFAULT_PUCK_DEPTH_OFFSET_MM		0

#define DEFAULT_ORIGIN_MARKER_ID			0
#define DEFAULT_UTILITY_MARKER_ID			16
#define DEFAULT_MARKER_SIZE_MM				100.f

#define DEFAULT_SPOUT_OUTPUT_NAME			"MikanXR"