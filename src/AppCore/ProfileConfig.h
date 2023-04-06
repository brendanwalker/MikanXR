#pragma once

// -- includes -----
#include "CommonConfig.h"
#include "ProfileConfigConstants.h"
#include "AnchorObjectSystem.h"
#include "StencilObjectSystem.h"
#include <filesystem>

// -- definitions -----
class ProfileConfig : public CommonConfig
{
public:
	ProfileConfig(const std::string& fnamebase = "ProfileConfig");

	virtual const configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	bool getSpatialAnchorInfo(MikanSpatialAnchorID anchorId, MikanSpatialAnchorInfo& outInfo) const;
	MikanSpatialAnchorID getNextSpatialAnchorId(MikanSpatialAnchorID anchorId) const;
	bool getSpatialAnchorWorldTransform(MikanSpatialAnchorID anchorId, glm::mat4& outXform) const;
	bool findSpatialAnchorInfoByName(const char* anchorName, MikanSpatialAnchorInfo& outInfo) const;
	bool canAddAnchor() const;
	bool addNewAnchor(const char* anchorName, const MikanMatrix4f& xform);
	bool updateAnchor(const MikanSpatialAnchorInfo& info);
	bool setAnchorName(MikanSpatialAnchorID anchorId, const std::string& newAnchorName);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

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

	bool canAddStencil() const;
	bool removeStencil(MikanStencilID stencilId);
	eStencilType getStencilType(MikanStencilID stencilId) const;
	bool getStencilName(MikanStencilID stencilId, std::string& outStencilName) const;
	bool getStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const;
	bool setStencilLocalTransform(MikanStencilID stencilId, const glm::mat4& xform);
	bool setStencilWorldTransform(MikanStencilID stencilId, const glm::mat4& xform);
	MikanSpatialAnchorID getStencilParentAnchorId(MikanStencilID stencilId) const;

	bool getQuadStencilInfo(MikanStencilID stencilId, MikanStencilQuad& outInfo) const;
	bool getQuadStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const;
	glm::mat4 getQuadStencilWorldTransform(const MikanStencilQuad* stencil) const;
	bool setQuadStencilLocalTransform(MikanStencilID stencilId, const glm::mat4& xform);
	bool setQuadStencilWorldTransform(MikanStencilID stencilId, const glm::mat4& xform);
	MikanStencilID addNewQuadStencil(const MikanStencilQuad& quad);
	bool updateQuadStencil(const MikanStencilQuad& info);

	bool getBoxStencilInfo(MikanStencilID stencilId, MikanStencilBox& outInfo) const;
	bool getBoxStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const;
	glm::mat4 getBoxStencilWorldTransform(const MikanStencilBox* stencil) const;
	bool setBoxStencilLocalTransform(MikanStencilID stencilId, const glm::mat4& xform);
	bool setBoxStencilWorldTransform(MikanStencilID stencilId, const glm::mat4& xform);
	MikanStencilID addNewBoxStencil(const MikanStencilBox& quad);
	bool updateBoxStencil(const MikanStencilBox& info);

	const MikanStencilModelConfig* getModelStencilConfig(MikanStencilID stencilId) const;
	bool getModelStencilInfo(MikanStencilID stencilId, MikanStencilModel& outInfo) const;
	bool getModelStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const;
	glm::mat4 getModelStencilWorldTransform(const MikanStencilModel* stencil) const;
	bool setModelStencilLocalTransform(MikanStencilID stencilId, const glm::mat4& xform);
	bool setModelStencilWorldTransform(MikanStencilID stencilId, const glm::mat4& xform);
	MikanStencilID addNewModelStencil(const MikanStencilModel& model);
	bool updateModelStencil(const MikanStencilModel& info);
	bool updateModelStencilFilename(MikanStencilID stencilID, const std::filesystem::path& filename);

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

	AnchorObjectSystemConfig anchorConfig;
	std::string anchorVRDevicePath;
	std::vector<MikanSpatialAnchorInfo> spatialAnchorList;
	MikanSpatialAnchorID nextAnchorId;
	MikanSpatialAnchorID originAnchorId;
	bool debugRenderAnchors;

	std::vector<MikanSpatialFastenerInfo> spatialFastenerList;
	MikanSpatialFastenerID nextFastenerId;
	bool debugRenderFasteners;

	StencilObjectSystemConfig stencilConfig;
	std::vector<MikanStencilQuad> quadStencilList;
	std::vector<MikanStencilBox> boxStencilList;
	std::vector<MikanStencilModelConfig> modelStencilList;
	MikanStencilID nextStencilId;
	bool debugRenderStencils;

	std::filesystem::path compositorScriptFilePath;
	std::filesystem::path outputFilePath;
};