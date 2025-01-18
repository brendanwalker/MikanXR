#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_VRTrackingRecenter.h"
#include "RendererFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

//-- definitions -----
class AppStage_VRTrackingRecenter : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_VRTrackingRecenter(class MainWindow* ownerWindow);
	virtual ~AppStage_VRTrackingRecenter();

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void setMenuState(eVRTrackingRecenterMenuState newState);

	// Calibration Model UI Events
	void onBeginEvent();
	void onRestartEvent();
	void onCancelEvent();
	void onReturnEvent();
	
private:
	class RmlModel_VRTrackingRecenter* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	bool m_bHasModifiedCameraSettings= false;

	VideoSourceViewPtr m_videoSourceView;

	// Tracking pucks used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	class ArucoMarkerPoseSampler* m_markerPoseSampler;
	class VideoFrameDistortionView* m_monoDistortionView;

	GlCameraPtr m_camera;
	GlFrameBufferPtr m_frameBuffer;
	GlTriangulatedMeshPtr m_fullscreenQuad;
};