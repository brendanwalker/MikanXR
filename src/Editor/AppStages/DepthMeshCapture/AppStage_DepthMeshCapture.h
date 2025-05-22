#pragma once

//-- includes -----
#include "AppStage.h"
#include "ComponentFwd.h"
#include "Constants_DepthMeshCapture.h"
#include "DeviceViewFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
#include "SceneFwd.h"

#include "glm/ext/matrix_float4x4.hpp"

#include <filesystem>
#include <memory>

class SyntheticDepthEstimator;
typedef std::shared_ptr<SyntheticDepthEstimator> SyntheticDepthEstimatorPtr;

class DepthMeshGenerator;
typedef std::shared_ptr<DepthMeshGenerator> DepthMeshGeneratorPtr;

class VideoFrameDistortionView;
typedef std::shared_ptr<VideoFrameDistortionView> VideoFrameDistortionViewPtr;

//-- definitions -----
class AppStage_DepthMeshCapture : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_DepthMeshCapture(class MainWindow* ownerWindow);
	virtual ~AppStage_DepthMeshCapture();

	void setTargetModelStencil(ModelStencilDefinitionPtr definition);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void setupCameras();
	MikanCameraPtr getViewpointCamera(eDepthMeshCaptureViewpointMode viewportMode) const;
	void renderVRScene();
	void setMenuState(eDepthMeshCaptureMenuState newState);

	// Calibration Model UI Events
	void onContinueEvent();
	void onRestartEvent();
	void onCancelEvent();

	// Camera Settings Model UI Events
	void onViewportModeChanged(eDepthMeshCaptureViewpointMode newViewMode);
	
	// GlScene Helpers
	void addDepthMeshResourcesToScene();
	void removeDepthMeshResourceFromScene();

private:
	ProjectConfigPtr m_profile;

	EditorObjectSystemPtr m_editorSystem;

	class RmlModel_DepthMeshCapture* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	class RmlModel_DepthMeshCameraSettings* m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView = nullptr;

	ModelStencilDefinitionPtr m_targetModelStencilDefinition;

	VideoSourceViewPtr m_videoSourceView;
	glm::mat4 m_videoSourceXform;

	VRDevicePoseViewPtr m_cameraTrackingPuckPoseView;

	DepthMeshGeneratorPtr m_depthMeshCapture;
	VideoFrameDistortionViewPtr m_monoDistortionView;
	SyntheticDepthEstimatorPtr m_syntheticDepthEstimator;

	std::vector<IMkStaticMeshInstancePtr> m_depthMeshInstances;
	IMkScenePtr m_mkScene;
	MikanViewportPtr m_viewport;
};