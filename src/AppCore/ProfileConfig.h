#pragma once

// -- includes -----
#include "CommonConfig.h"
#include "ObjectSystemConfigFwd.h"
#include "ProfileConfigConstants.h"

#include <filesystem>

// -- definitions -----
class ProfileConfig : public CommonConfig
{
public:
	ProfileConfig(const std::string& fnamebase = "ProfileConfig");

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	std::filesystem::path generateTimestampedFilePath(const std::string& prefix, const std::string& suffix) const;

	eCalibrationPatternType calibrationPatternType;
	int chessbordRows;
	int chessbordCols;
	float squareLengthMM;
	int circleGridRows;
	int circleGridCols;
	float circleSpacingMM;
	float circleDiameterMM;
	float puckHorizontalOffsetMM;
	float puckVerticalOffsetMM;
	float puckDepthOffsetMM;

	std::string videoSourcePath;

	static const std::string k_cameraVRDevicePathPropertyId;
	std::string cameraVRDevicePath;

	static const std::string k_cameraParentAnchorPropertyId;
	MikanSpatialAnchorID cameraParentAnchorId;

	static const std::string k_cameraScalePropertyId;
	float cameraScale;

	static const std::string k_matVRDevicePathPropertyId;
	std::string matVRDevicePath;

	static const std::string k_originVRDevicePathPropertyId;
	std::string originVRDevicePath;

	static const std::string k_originVerticalAlignFlagPropertyId;
	bool originVerticalAlignFlag;

	std::string calibrationComponentName;
	int vrFrameDelay;
	int videoFrameQueueSize;

	std::filesystem::path compositorScriptFilePath;
	std::filesystem::path outputFilePath;

	AnchorObjectSystemConfigPtr anchorConfig;
	EditorObjectSystemConfigPtr editorConfig;
	FastenerObjectSystemConfigPtr fastenerConfig;
	StencilObjectSystemConfigPtr stencilConfig;
};