#pragma once

#include "AppStage.h"
#include "Constants_SceneLightingCapture.h"
#include "LightSystemFwd.h"
#include "MikanCoreTypes.h"
#include "MikanRendererFwd.h"
#include "MikanTypeFwd.h"
#include "SceneLightingEstimator.h"
#include "VideoDisplayConstants.h"

#include <memory>
#include <string>

class GuiPanel_SceneLightingCapture;

/// Captures one video frame and recovers the scene's low-frequency lighting
/// into a LightEnvironmentComponent.
///
/// The estimate is judged before it is committed: the panel shows the
/// directionality confidence signal and a lit sphere rendered from the
/// recovered environment, so an operator can compare it against the plate
/// rather than trusting a number. See docs/reference/scene-lighting.md.
class AppStage_SceneLightingCapture : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_SceneLightingCapture(class IEditorWindow* ownerWindow);
	virtual ~AppStage_SceneLightingCapture();

	/// Probe the recovered environment is written into. Call before enter().
	void setTargetProbe(LightEnvironmentComponentPtr targetProbe);

	/// Camera supplying the video feed and the pose used to rotate the
	/// estimate out of camera space. Call before enter().
	void setSourceCamera(CameraComponentPtr cameraComponent);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	// Required: AppStage::onGui() only draws the modal dialog stack, so a stage
	// that does not override this renders no panels at all.
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void setMenuState(eSceneLightingCaptureMenuState newState);

	void runCapture();
	void applyEstimate();

	/// Draws a sphere lit by the recovered environment so the estimate can be
	/// judged visually.
	void renderLitSpherePreview();

	// GUI button events
	void onCaptureEvent();
	void onApplyEvent();
	void onRedoEvent();
	void onCancelEvent();

private:
	class GuiPanel_SceneLightingCapture* m_capturePanel= nullptr;

	CameraComponentPtr m_currentSceneCameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;
	class VideoFrameDistortionView* m_monoDistortionView= nullptr;

	LightEnvironmentComponentPtr m_targetProbe;

	// Owned here rather than by the object system: the ONNX models are several
	// gigabytes, so they are loaded when the capture tool is opened and freed
	// when it closes.
	std::unique_ptr<SceneLightingEstimator> m_estimator;
	SceneLightingEstimator::Result m_result;
	bool m_bHasResult= false;

	MikanCameraPtr m_mkCamera;
};
