#pragma once

//-- includes -----
#include "AppStage.h"
#include "AnchorTriangulator.h"
#include "Constants_AnchorTriangulation.h"
#include "MikanRendererFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

class GuiPanel_AnchorTriangulation;

//-- definitions -----
class AppStage_AnchorTriangulation : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_AnchorTriangulation(class IEditorWindow* ownerWindow);
	virtual ~AppStage_AnchorTriangulation();

	void setBypassCalibrationFlag(bool flag);
	void setSourceCamera(CameraComponentPtr cameraComponent);
	void setTargetAnchor(const AnchorTriangulatorInfo& anchor) { m_targetAnchor= anchor; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void onVideoSourceReady(VideoSourceComponentPtr videoSourceComponent);
	void setupDistortionView();
	void updateCamera();
	void setMenuState(eAnchorTriangulationMenuState newState);

	// Input Events
	void onMouseButtonUp(int button);

	// Calibration Model UI Events
	void onOkEvent();
	void onRedoEvent();
	void onCancelEvent();

private:
	class GuiPanel_AnchorTriangulation* m_calibrationPanel = nullptr;

	CameraComponentPtr m_currentSceneCameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;

	AnchorTriangulator* m_anchorTriangulator;
	class VideoFrameDistortionView* m_monoDistortionView;

	AnchorTriangulatorInfo m_targetAnchor;

	bool m_bypassCalibrationFlag = false;
	MikanCameraPtr m_mkCamera;
};