#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_DepthMeshCapture.h"
#include "RendererFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

//-- definitions -----
class AppStage_DepthMeshCapture : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_DepthMeshCapture(class MainWindow* ownerWindow);
	virtual ~AppStage_DepthMeshCapture();

	void setBypassCalibrationFlag(bool flag);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void updateCamera();
	void renderVRScene();
	void setMenuState(eDepthMeshCaptureMenuState newState);

	// Calibration Model UI Events
	void onBeginEvent();
	void onRestartEvent();
	void onCancelEvent();
	void onReturnEvent();

	// Camera Settings Model UI Events
	void onViewportModeChanged(eDepthMeshCaptureViewpointMode newViewMode);
	
private:
	class RmlModel_DepthMeshCapture* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	class RmlModel_DepthMeshCameraSettings* m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView = nullptr;

	VideoSourceViewPtr m_videoSourceView;

	// Tracking pucks used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	class MonoLensDepthMeshCapture* m_depthMeshCapture;
	class VideoFrameDistortionView* m_monoDistortionView;

	GlScenePtr m_scene;
	GlCameraPtr m_camera;
};