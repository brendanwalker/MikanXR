#pragma once

//-- includes -----
#include "AppStage.h"
#include "ComponentFwd.h"
#include "Constants_AlignmentCalibration.h"
#include "MikanRendererFwd.h"
#include "VideoDisplayConstants.h"
#include "VRDevicePoseView.h"

#include <memory>

class GuiPanel_AlignmentCalibration;
class GuiPanel_AlignmentCameraSettings;

//-- definitions -----
class AppStage_AlignmentCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_AlignmentCalibration(class IEditorWindow* ownerWindow);
	virtual ~AppStage_AlignmentCalibration();
	
	static bool tryEnterAlignmentCalibration(
		AppStage* fromAppStage,
		CameraComponentPtr forCameraComponent);

	void setBypassCalibrationFlag(bool flag);
	void setTargetCameraComponent(CameraComponentPtr cameraComponent);
	void setVideoSourceComponent(VideoSourceComponentPtr videoSourceComponent);
	void setCameraPuckPose(VRDevicePoseViewPtr cameraPuckPose);
	void setMatPuckPose(VRDevicePoseViewPtr matPuckPose);

	// -- AppStage -- //
	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void setupTrackerPoseCalibrator();
	void updateCameraTransform();
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
	class GuiPanel_AlignmentCalibration* m_calibrationPanel = nullptr;
	class GuiPanel_AlignmentCameraSettings* m_cameraSettingsPanel = nullptr;

	bool m_bypassCalibrationFlag = false;
	CameraComponentPtr m_targetCameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;

	// Tracking pucks used for calibration
	VRDevicePoseViewPtr m_cameraPuckPose_VRSystemSpace;
	VRDevicePoseViewPtr m_matPuckPose_VRSystemSpace;

	class MonoLensTrackerPoseCalibrator* m_trackerPoseCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;

	MkScenePtr m_scene;
	MikanCameraPtr m_mkCamera;
	IMkFrameBufferPtr m_frameBuffer;
	IMkTriangulatedMeshPtr m_fullscreenRGBQuad;
};