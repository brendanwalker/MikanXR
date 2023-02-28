#pragma once

// -- includes -----
#include "CommonConfig.h"
#include "ProfileConfigConstants.h"
#include <filesystem>

struct MikanStencilModelConfig
{
	MikanStencilModel modelInfo;
	std::filesystem::path modelPath;
};

// -- definitions -----
class ProfileConfig : public CommonConfig
{
public:
	ProfileConfig(const std::string& fnamebase = "ProfileConfig");

	virtual const configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	bool getSpatialAnchorInfo(MikanSpatialAnchorID anchorId, MikanSpatialAnchorInfo& outInfo) const;
	MikanSpatialAnchorID getNextSpatialAnchorId(MikanSpatialAnchorID anchorId) const;
	bool findSpatialAnchorInfoByName(const char* anchorName, MikanSpatialAnchorInfo& outInfo) const;
	bool canAddAnchor() const;
	bool addNewAnchor(const char* anchorName, const MikanMatrix4f& xform);
	bool updateAnchor(const MikanSpatialAnchorInfo& info);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

	bool canAddStencil() const;
	bool removeStencil(MikanStencilID stencilId);
	eStencilType getStencilType(MikanStencilID stencilId) const;
	bool getStencilName(MikanStencilID stencilId, std::string& outStencilName) const;

	bool getQuadStencilInfo(MikanStencilID stencilId, MikanStencilQuad& outInfo) const;
	glm::mat4 getQuadStencilWorldTransform(const MikanStencilQuad* stencil) const;
	MikanStencilID addNewQuadStencil(const MikanStencilQuad& quad);
	bool updateQuadStencil(const MikanStencilQuad& info);

	bool getBoxStencilInfo(MikanStencilID stencilId, MikanStencilBox& outInfo) const;
	glm::mat4 getBoxStencilWorldTransform(const MikanStencilBox* stencil) const;
	MikanStencilID addNewBoxStencil(const MikanStencilBox& quad);
	bool updateBoxStencil(const MikanStencilBox& info);

	bool getModelStencilInfo(MikanStencilID stencilId, MikanStencilModel& outInfo) const;
	glm::mat4 getModelStencilWorldTransform(const MikanStencilModel* stencil) const;
	MikanStencilID addNewModelStencil(const MikanStencilModel& model);
	bool updateModelStencil(const MikanStencilModel& info);
	bool updateModelStencilFilename(MikanStencilID stencilID, const std::string& filename);

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
	std::string calibrationComponentName;
	int vrFrameDelay;
	int videoFrameQueueSize;

	std::string anchorVRDevicePath;
	std::vector<MikanSpatialAnchorInfo> spatialAnchorList;
	MikanSpatialAnchorID nextAnchorId;

	std::vector<MikanStencilQuad> quadStencilList;
	std::vector<MikanStencilBox> boxStencilList;
	std::vector<MikanStencilModelConfig> modelStencilList;
	MikanStencilID nextStencilId;

	std::filesystem::path compositorScriptFilePath;
	std::filesystem::path outputFilePath;
};