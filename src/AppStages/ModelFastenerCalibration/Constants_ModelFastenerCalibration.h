#pragma once

#include <string>

enum class eModelFastenerCalibrationMenuState : int
{
	INVALID = -1,

	inactive,
	verifyModel,
	captureModelVertices,
	verifyVerticesCapture,

	COUNT
};
extern const std::string* k_modelFastenerCalibrationMenuStateStrings;