#pragma once

//-- includes -----
#include "AppStage.h"
#include "ComponentFwd.h"
#include "Constants_MonoLensCalibration.h"
#include "VideoDisplayConstants.h"
#include <memory>

class GuiPanel_MonoLensCalibration;
class GuiPanel_MonoCameraSettings;

//-- definitions -----
class AppStage_MonoLensCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_MonoLensCalibration(class IEditorWindow* ownerWindow);
	virtual ~AppStage_MonoLensCalibration();

	void setBypassCalibrationFlag(bool flag);
	void setVideoSourceComponent(VideoSourceComponentPtr videoSourceComponent);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void setMenuState(eMonoLensCalibrationMenuState newState);
	void setupMonoLensCalibrator();
	void onCaptureKeyPressed();
	bool tryCapture();

	// Calibration Model UI Events
	void onRestartEvent();
	void onReturnEvent();
	void onCancelEvent();
	void onImagePointStabilityChangedEvent(bool areImagePointsStable);

	// Camera Settings Model UI Events
	void onVideoDisplayModeChanged(eVideoDisplayMode newDisplayMode);

	// Remote Control
	virtual bool handleRemoteControlCommand(const std::string& command, const std::vector<std::string>& parameters,
											std::vector<std::string>& outResults) override;
	bool handleGetStateCommand(std::vector<std::string>& outResults);
	bool handleGetImagePointStabilityCommand(std::vector<std::string>& outResults);
	bool handleGetSamplesNeededCommand(std::vector<std::string>& outResults);
	bool handleCaptureCommand(std::vector<std::string>& outResults);

private:
	class GuiPanel_MonoLensCalibration* m_calibrationPanel= nullptr;
	class GuiPanel_MonoCameraSettings* m_cameraSettingsPanel= nullptr;

	bool m_bypassCalibrationFlag= false;
	VideoSourceComponentPtr m_videoSourceComponent;
	class MonoLensDistortionCalibrator* m_monoLensCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;
};