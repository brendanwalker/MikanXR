#pragma once

//-- includes -----
#include "AppStage.h"
#include "ComponentFwd.h"
#include "DeviceViewFwd.h"
#include "Constants_VRTrackingRecenter.h"
#include "IRemoteControllableAppStage.h"
#include "MikanTypeFwd.h"
#include "MikanRendererFwd.h"
#include "MikanCoreTypes.h"
#include "VideoDisplayConstants.h"

#include <memory>

//-- definitions -----
class AppStage_VRTrackingRecenter : 
	public AppStage,
	public IRemoteControllableAppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_VRTrackingRecenter(class MainWindow* ownerWindow);
	virtual ~AppStage_VRTrackingRecenter();

	inline void setTargetStageId(MikanStageID stageId) { m_targetStageId = stageId; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void setMenuState(eVRTrackingRecenterMenuState newState);
	void updateCameraPose();

	// Calibration Model UI Events
	void onBeginEvent();
	void onRestartEvent();
	void onCancelEvent();
	void onReturnEvent();
	void onMarkerStabilityChangedEvent(bool bIsStable);

	bool tryBeginCapture();
	bool tryRestartCapture();

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
	class RmlModel_VRTrackingRecenter* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	MikanStageID m_targetStageId = INVALID_MIKAN_ID;
	bool m_bHasModifiedCameraSettings= false;

	CameraComponentPtr m_cameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;

	class ArucoMarkerPoseSampler* m_markerPoseSampler;
	class VideoFrameDistortionView* m_monoDistortionView;

	MikanCameraPtr m_mkCamera;
	IMkFrameBufferPtr m_frameBuffer;
	IMkTriangulatedMeshPtr m_fullscreenQuad;
};