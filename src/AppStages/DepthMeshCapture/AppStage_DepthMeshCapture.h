#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_DepthMeshCapture.h"
#include "RendererFwd.h"
#include "VideoDisplayConstants.h"

#include <filesystem>
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

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

	void setMeshSavePath(const std::filesystem::path& path) { m_meshSavePath = path; }
	void setTextureSavePath(const std::filesystem::path& path) { m_textureSavePath = path; }
	void setBypassCaptureFlag(bool flag);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void updateCamera();
	void renderVRScene();
	void setMenuState(eDepthMeshCaptureMenuState newState);

	// Calibration Model UI Events
	void onContinueEvent();
	void onRestartEvent();
	void onCancelEvent();

	// Camera Settings Model UI Events
	void onViewportModeChanged(eDepthMeshCaptureViewpointMode newViewMode);
	
private:
	std::filesystem::path m_meshSavePath;
	std::filesystem::path m_textureSavePath;

	class RmlModel_DepthMeshCapture* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	class RmlModel_DepthMeshCameraSettings* m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView = nullptr;

	VideoSourceViewPtr m_videoSourceView;

	// Tracking pucks used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	DepthMeshGeneratorPtr m_depthMeshCapture;
	VideoFrameDistortionViewPtr m_monoDistortionView;
	SyntheticDepthEstimatorPtr m_syntheticDepthEstimator;

	GlScenePtr m_scene;
	GlCameraPtr m_camera;
};