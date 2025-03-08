#pragma once

//-- includes -----
#include "AppStage.h"
#include "DeviceViewFwd.h"
#include "Constants_AlignmentCalibration.h"
#include "IRemoteControllableAppStage.h"
#include "MikanRendererFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

//-- definitions -----
class AppStage_AlignmentCalibration : 
	public AppStage, 
	public IRemoteControllableAppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_AlignmentCalibration(class MainWindow* ownerWindow);
	virtual ~AppStage_AlignmentCalibration();

	void setBypassCalibrationFlag(bool flag);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void updateCamera();
	void renderVRScene();
	void setMenuState(eAlignmentCalibrationMenuState newState);

	// Calibration Model UI Events
	void onBeginEvent();
	void onRestartEvent();
	void onCancelEvent();
	void onReturnEvent();
	void onChessboardStabilityChangedEvent(bool bIsStable);

	bool tryBeginCapture();
	bool tryRestartCapture();

	// Camera Settings Model UI Events
	void onViewportModeChanged(eAlignmentCalibrationViewpointMode newViewMode);
	void onBrightnessChanged(int newBrightness);
	void onVRFrameDelayChanged(int newVRFrameDelay);

	// Remote Control
	virtual bool handleRemoteControlCommand(
		const std::string& command,
		const std::vector<std::string>& parameters,
		std::vector<std::string>& outResults) override;
	bool handleGetStateCommand(std::vector<std::string>& outResults);
	bool handleGetChessboardStabilityCommand(std::vector<std::string>& outResults);
	bool handleBeginCommand(std::vector<std::string>& outResults);
	bool handleRestartCommand(std::vector<std::string>& outResults);
	
private:
	class RmlModel_AlignmentCalibration* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	class RmlModel_AlignmentCameraSettings* m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView = nullptr;

	bool m_bHasModifiedCameraSettings= false;

	VideoSourceViewPtr m_videoSourceView;

	// Tracking pucks used for calibration
	VRDevicePoseViewPtr m_cameraTrackingPuckPoseView;
	VRDevicePoseViewPtr m_matTrackingPuckPoseView;

	class MonoLensTrackerPoseCalibrator* m_trackerPoseCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;

	GlScenePtr m_scene;
	MikanCameraPtr m_camera;
	IMkFrameBufferPtr m_frameBuffer;
	IMkTriangulatedMeshPtr m_fullscreenQuad;
};