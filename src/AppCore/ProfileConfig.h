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

#if 0
	bool getSpatialFastenerInfo(MikanSpatialFastenerID fastenerId, MikanSpatialFastenerInfo& outInfo) const;
	bool getFastenerWorldTransform(MikanSpatialFastenerID fastenerId, glm::mat4& outXform) const;
	glm::mat4 getFastenerWorldTransform(const MikanSpatialFastenerInfo* fastener) const;
	void getFastenerLocalPoints(const MikanSpatialFastenerInfo* fastener, glm::vec3 outLocalPoints[3]) const;
	void getFastenerWorldPoints(const MikanSpatialFastenerInfo* fastener, glm::vec3 outWorldPoints[3]) const;
	MikanSpatialFastenerID getNextSpatialFastenerId(MikanSpatialFastenerID fastenerId) const;
	bool findSpatialFastenerInfoByName(const char* fastenerName, MikanSpatialFastenerInfo& outInfo) const;
	std::vector<MikanSpatialFastenerID> getSpatialFastenersWithParent(const MikanFastenerParentType parentType, const int32_t objectId) const;
	std::vector<MikanSpatialFastenerID> getValidSpatialFastenerSnapTargets(const MikanSpatialFastenerID sourceFastenerId) const;
	bool canAddFastener() const;
	MikanSpatialFastenerID addNewFastener(const MikanSpatialFastenerInfo& fastener);
	bool updateFastener(const MikanSpatialFastenerInfo& fastener);
	bool removeFastener(MikanSpatialFastenerID fastenerId);
#endif

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

	std::string cameraVRDevicePath;
	MikanSpatialAnchorID cameraParentAnchorId;
	float cameraScale;
	std::string matVRDevicePath;
	std::string originVRDevicePath;
	bool originVerticalAlignFlag;
	std::string calibrationComponentName;
	int vrFrameDelay;
	int videoFrameQueueSize;

	std::vector<MikanSpatialFastenerInfo> spatialFastenerList;
	MikanSpatialFastenerID nextFastenerId;
	bool debugRenderFasteners;

	std::filesystem::path compositorScriptFilePath;
	std::filesystem::path outputFilePath;

	AnchorObjectSystemConfigPtr anchorConfig;
	StencilObjectSystemConfigPtr stencilConfig;
};