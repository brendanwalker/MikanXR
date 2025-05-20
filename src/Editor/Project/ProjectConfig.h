#pragma once

// -- includes -----
#include "CommonConfig.h"
#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "ProjectConfigConstants.h"

#include <filesystem>

// -- definitions -----
class ProjectConfig : public CommonConfig
{
public:
	ProjectConfig(const std::string& fnamebase = "ProfileConfig");

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	std::filesystem::path generateTimestampedFilePath(const std::string& prefix, const std::string& suffix) const;

	eCalibrationPatternType calibrationPatternType;
	int chessbordRows;
	int chessbordCols;
	float squareLengthMM;
	int charucoRows;
	int charucoCols;
	float charucoSquareLengthMM;
	float charucoMarkerLengthMM;
	eCharucoDictionaryType charucoDictionaryType;
	float puckHorizontalOffsetMM;
	float puckVerticalOffsetMM;
	float puckDepthOffsetMM;
	int vrCenterArucoId;
	float vrCenterMarkerLengthMM;

	static const std::string k_spoutOutputIsStreamingNamePropertyId;
	inline bool getIsSpoutOutputStreaming() const { return m_bIsSpoutOutputStreaming; }
	void setIsSpoutOutputStreaming(bool bIsStreaming);

	static const std::string k_spoutOutputNamePropertyId;
	inline const std::string& getSpoutOutputName() const { return m_spoutOutputName; }
	void setSpoutOutputName(const std::string& spoutOutputName);

	static const std::string k_videoSourcePathPropertyId;
	std::string videoSourcePath;

	static const std::string k_cameraVRDevicePathPropertyId;
	std::string cameraVRDevicePath;

	static const std::string k_matVRDevicePathPropertyId;
	std::string matVRDevicePath;

	static const std::string k_vrDevicePoseOffsetPropertyId;
	MikanMatrix4f vrDevicePoseOffset;

	static const std::string k_renderOriginFlagPropertyId;
	inline bool getRenderOriginFlag() const { return m_bRenderOrigin; }
	void setRenderOriginFlag(bool flag);

	static const std::string k_vrFrameDelayPropertyId;
	inline int getVRFrameDelay() const { return m_vrFrameDelay; }
	void setVRFrameDelay(int frameDelay);

	std::string vivePuckDefaultComponentName;
	int videoFrameQueueSize;

	std::filesystem::path compositorScriptFilePath;
	std::filesystem::path outputFilePath;

	AnchorObjectSystemConfigPtr anchorConfig;
	EditorObjectSystemConfigPtr editorConfig;
	StencilObjectSystemConfigPtr stencilConfig;
	SceneObjectSystemConfigPtr sceneConfig;
	CameraObjectSystemConfigPtr cameraConfig;

protected:
	bool m_bIsSpoutOutputStreaming= false;
	std::string m_spoutOutputName;
	bool m_bRenderOrigin= true;
	int m_vrFrameDelay = 0;
};