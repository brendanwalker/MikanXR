#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_ModelFastenerCalibration.h"
#include "MikanClientTypes.h"
#include "RendererFwd.h"
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

	void setTargetFastener(const MikanSpatialFastenerInfo& fastener) { m_targetFastener= fastener; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void updateCamera();
	void updateClosestModelVertex();
	void renderModelScene() const;
	void renderClosestModelVertex() const;
	void renderCaptureModelVertices() const;
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

	MikanSpatialFastenerInfo m_targetFastener;
	struct ModelFastenerCalibrationState* m_calibrationState;

	GlRenderModelResourcePtr m_modelResource;
};