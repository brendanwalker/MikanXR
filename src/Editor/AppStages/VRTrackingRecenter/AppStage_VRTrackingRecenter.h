#pragma once

//-- includes -----
#include "AppStage.h"
#include "ComponentFwd.h"
#include "Constants_VRTrackingRecenter.h"
#include "MikanTypeFwd.h"
#include "MikanRendererFwd.h"
#include "MikanCoreTypes.h"
#include "VideoDisplayConstants.h"

#include <memory>

class GuiPanel_VRTrackingRecenter;

//-- definitions -----
class AppStage_VRTrackingRecenter : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_VRTrackingRecenter(class IEditorWindow* ownerWindow);
	virtual ~AppStage_VRTrackingRecenter();

	static bool tryEnterAlignmentCalibration(class AppStage* fromAppStage, CameraComponentPtr withCameraComponent,
											 VRTrackingVolumeComponentPtr forTrackingVolume);

	void setSourceCamera(CameraComponentPtr cameraComponent);
	inline void setTargetVRTrackingVolume(VRTrackingVolumeComponentPtr trackingVolume)
	{
		m_targetTrackingVolume= trackingVolume;
	}

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void setMenuState(eVRTrackingRecenterMenuState newState);
	void setupMarkerPoseSampler();
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
	virtual bool handleRemoteControlCommand(const std::string& command, const std::vector<std::string>& parameters,
											std::vector<std::string>& outResults) override;
	bool handleGetStateCommand(std::vector<std::string>& outResults);
	bool handleGetChessboardStabilityCommand(std::vector<std::string>& outResults);
	bool handleBeginCommand(std::vector<std::string>& outResults);
	bool handleRestartCommand(std::vector<std::string>& outResults);

private:
	class GuiPanel_VRTrackingRecenter* m_calibrationPanel= nullptr;

	CameraComponentPtr m_cameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;
	VRTrackingVolumeComponentPtr m_targetTrackingVolume;

	class ArucoMarkerPoseSampler* m_markerPoseSampler;
	class VRDevicePoseSampler* m_puckSampler= nullptr;
	class VideoFrameDistortionView* m_monoDistortionView;

	MikanCameraPtr m_mkCamera;
	IMkFrameBufferPtr m_frameBuffer;
	IMkTriangulatedMeshPtr m_fullscreenRGBQuad;
};