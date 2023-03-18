#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_ModelFastenerCalibration.h"
#include "MikanClientTypes.h"
#include "VideoDisplayConstants.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

//-- definitions -----
class AppStage_ModelFastenerCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_ModelFastenerCalibration(class App* app);
	virtual ~AppStage_ModelFastenerCalibration();

	void setTargetFastenerId(MikanSpatialFastenerID fastenerId) { m_targetFastenerId= fastenerId; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update() override;
	virtual void render() override;

protected:
	void updateCamera();
	void renderModelScene();
	void setMenuState(eModelFastenerCalibrationMenuState newState);

	// Input Events
	void onMouseButtonUp(int button);

	// Calibration Model UI Events
	void onOkEvent();
	void onRedoEvent();
	void onCancelEvent();

private:
	class RmlModel_ModelFastenerCalibration* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	// Tracking puck used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	MikanSpatialFastenerID m_targetFastenerId;

	class GlScene* m_scene;
	class GlCamera* m_camera;
	const class GlRenderModelResource* m_modelResource;
};