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

	template<class t_system_config_type, class t_system_type>
	std::shared_ptr<t_system_config_type> addTypedDefinition()
	{
		std::shared_ptr<t_system_config_type> newConfig =
			std::make_shared<t_system_config_type>(
				t_system_type::k_objectSystemClassName);

		addChildConfig(newConfig);
		return newConfig;
	}
	MikanObjectSystemDefinitionPtr getDefinitionForSystem(MikanObjectSystemPtr systemPtr) const;

	AnchorObjectSystemConfigPtr anchorConfig;
	BoxStencilSystemDefinitionPtr boxStencilSystemDefinition;
	CameraObjectSystemConfigPtr cameraConfig;
	ClientTextureSourceSystemConfigPtr clientConfig;
	CompositorObjectSystemConfigPtr compositorConfig;
	EditorObjectSystemConfigPtr editorConfig;
	MarkerObjectSystemConfigPtr markerSystemConfig;
	MarkerTrackingVolumeSystemDefinitionPtr markerTrackingVolumeConfig;
	ModelStencilSystemDefinitionPtr modelStencilSystemDefinition;
	QuadStencilSystemDefinitionPtr quadStencilSystemDefinition;
	SceneObjectSystemDefinitionPtr sceneConfig;
	SpoutTextureSourceSystemConfigPtr spoutConfig;
	StageObjectSystemConfigPtr stageConfig;
	TrackingMountObjectSystemConfigPtr trackingMountSystemConfig;
	NetworkVideoSourceSystemConfigPtr networkVideoSourceSystemConfig;
	USBVideoSourceSystemConfigPtr usbVideoSourceSystemConfig;
	VRObjectSystemConfigPtr vrObjectConfig;
	VRTrackingVolumeSystemDefinitionPtr vrTrackingVolumeConfig;

protected:
	eTrackingRuntime m_trackingRuntime= eTrackingRuntime::SteamVR;
	bool m_bRenderOrigin= true;
};