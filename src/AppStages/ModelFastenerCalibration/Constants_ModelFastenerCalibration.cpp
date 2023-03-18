#include "Constants_ModelFastenerCalibration.h"

const std::string g_modelFastenerCalibrationMenuStateStrings[(int)eModelFastenerCalibrationMenuState::COUNT] = {
	"inactive",
	"verifyModel",
	"captureModelVertices",
	"verifyVerticesCapture",
};
const std::string* k_modelFastenerCalibrationMenuStateStrings = g_modelFastenerCalibrationMenuStateStrings;