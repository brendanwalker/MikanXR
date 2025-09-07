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


	static const std::string k_renderOriginFlagPropertyId;
	inline bool getRenderOriginFlag() const { return m_bRenderOrigin; }
	void setRenderOriginFlag(bool flag);

	std::filesystem::path compositorScriptFilePath;

	AnchorObjectSystemConfigPtr anchorConfig;
	CameraObjectSystemConfigPtr cameraConfig;
	CompositorObjectSystemConfigPtr compositorConfig;
	EditorObjectSystemConfigPtr editorConfig;
	MarkerObjectSystemConfigPtr markerSystemConfig;
	StencilObjectSystemConfigPtr stencilConfig;
	SceneObjectSystemConfigPtr sceneConfig;
	StageObjectSystemConfigPtr stageConfig;
	TrackingSystemsConfigPtr trackingSystemsConfig;
	VideoSourceSystemConfigPtr videoSourceSystemConfig;
	VRObjectSystemConfigPtr vrObjectConfig;

protected:
	eTrackingRuntime m_trackingRuntime= eTrackingRuntime::SteamVR;
	bool m_bRenderOrigin= true;
};