#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_MonoLensCalibration.h"
#include "IRemoteControllableAppStage.h"
#include "RmlFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

//-- definitions -----
class AppStage_MonoLensCalibration : 
	public AppStage,
	public IRemoteControllableAppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_MonoLensCalibration(class MainWindow* ownerWindow);
	virtual ~AppStage_MonoLensCalibration();

	void setBypassCalibrationFlag(bool flag);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void setMenuState(eMonoLensCalibrationMenuState newState);
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
	virtual bool handleRemoteControlCommand(
		const std::string& command,
		const std::vector<std::string>& parameters,
		std::vector<std::string>& outResults) override;
	bool handleGetStateCommand(std::vector<std::string>& outResults);
	bool handleGetImagePointStabilityCommand(std::vector<std::string>& outResults);
	bool handleGetSamplesNeededCommand(std::vector<std::string>& outResults);
	bool handleCaptureCommand(std::vector<std::string>& outResults);

private:
	class RmlModel_MonoLensCalibration* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView= nullptr;

	class RmlModel_MonoCameraSettings* m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView= nullptr;

	VideoSourceViewPtr m_videoSourceView;
	class MonoLensDistortionCalibrator* m_monoLensCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;
};