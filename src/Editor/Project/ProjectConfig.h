#pragma once

// -- includes -----
#include "CommonConfig.h"
#include "MikanMathTypes.h"
#include "ObjectSystemFwd.h"
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

	MikanObjectSystemDefinitionPtr getDefinitionForSystem(MikanObjectSystemPtr systemPtr) const;

	AnchorObjectSystemConfigPtr anchorConfig;
	CameraObjectSystemConfigPtr cameraConfig;
	CompositorObjectSystemConfigPtr compositorConfig;
	EditorObjectSystemConfigPtr editorConfig;
	MarkerObjectSystemConfigPtr markerSystemConfig;
	StencilObjectSystemConfigPtr stencilConfig;
	SceneObjectSystemDefinitionPtr sceneConfig;
	StageObjectSystemConfigPtr stageConfig;
	TrackingVolumeObjectSystemConfigPtr trackingVolumeSystemConfig;
	TrackingMountObjectSystemConfigPtr trackingMountSystemConfig;
	TextureSourceSystemConfigPtr textureSourceSystemConfig;
	VideoSourceSystemConfigPtr videoSourceSystemConfig;
	VRObjectSystemConfigPtr vrObjectConfig;

protected:
	eTrackingRuntime m_trackingRuntime= eTrackingRuntime::SteamVR;
	bool m_bRenderOrigin= true;
};