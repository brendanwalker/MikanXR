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

	static const std::string k_spoutOutputIsStreamingNamePropertyId;
	inline bool getIsSpoutOutputStreaming() const { return m_bIsSpoutOutputStreaming; }
	void setIsSpoutOutputStreaming(bool bIsStreaming);

	static const std::string k_spoutOutputNamePropertyId;
	inline const std::string& getSpoutOutputName() const { return m_spoutOutputName; }
	void setSpoutOutputName(const std::string& spoutOutputName);

	static const std::string k_renderOriginFlagPropertyId;
	inline bool getRenderOriginFlag() const { return m_bRenderOrigin; }
	void setRenderOriginFlag(bool flag);

	std::filesystem::path compositorScriptFilePath;

	AnchorObjectSystemConfigPtr anchorConfig;
	CameraObjectSystemConfigPtr cameraConfig;
	CompositorObjectSystemConfigPtr compositorConfig;
	EditorObjectSystemConfigPtr editorConfig;
	MarkerSystemConfigPtr markerSystemConfig;
	StencilObjectSystemConfigPtr stencilConfig;
	SceneObjectSystemConfigPtr sceneConfig;
	StageObjectSystemConfigPtr stageConfig;
	TrackingSystemsConfigPtr trackingSystemsConfig;
	VideoSourceSystemConfigPtr videoSourceSystemConfig;
	VRObjectSystemConfigPtr vrObjectConfig;

protected:
	bool m_bIsSpoutOutputStreaming= false;
	std::string m_spoutOutputName;
	eTrackingRuntime m_trackingRuntime= eTrackingRuntime::SteamVR;
	bool m_bRenderOrigin= true;
};