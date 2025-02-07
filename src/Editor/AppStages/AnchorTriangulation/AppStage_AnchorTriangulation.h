#pragma once

//-- includes -----
#include "AppStage.h"
#include "AnchorTriangulator.h"
#include "Constants_AnchorTriangulation.h"
#include "DeviceViewFwd.h"
#include "MikanRendererFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

//-- definitions -----
class AppStage_AnchorTriangulation : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_AnchorTriangulation(class MainWindow* ownerWindow);
	virtual ~AppStage_AnchorTriangulation();

	void setBypassCalibrationFlag(bool flag);
	void setTargetAnchor(const AnchorTriangulatorInfo& anchor) { m_targetAnchor= anchor; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void updateCamera();
	void setMenuState(eAnchorTriangulationMenuState newState);

	// Input Events
	void onMouseButtonUp(int button);

	// Calibration Model UI Events
	void onOkEvent();
	void onRedoEvent();
	void onCancelEvent();

private:
	class RmlModel_AnchorTriangulation* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	VideoSourceViewPtr m_videoSourceView;

	// Tracking puck used for calibration
	VRDevicePoseViewPtr m_cameraTrackingPuckPoseView;

	AnchorTriangulator* m_anchorTriangulator;
	class VideoFrameDistortionView* m_monoDistortionView;

	AnchorTriangulatorInfo m_targetAnchor;

	GlCameraPtr m_camera;
};