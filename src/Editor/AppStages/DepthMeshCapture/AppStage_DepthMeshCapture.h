#pragma once

#include "AppStage.h"
#include "Constants_DepthMeshCapture.h"
#include "DepthMeshGenerator.h"
#include "MikanRendererFwd.h"
#include "MikanTypeFwd.h"
#include "MoGeInference.h"
#include "VideoDisplayConstants.h"

#include <memory>
#include <string>

class GuiPanel_DepthMeshCapture;

/// Captures one video frame, recovers metric scene depth from it, and turns
/// the result into a model stencil (shadow catcher) placed at the capturing
/// camera's pose - replacing hand-placed proxy geometry.
///
/// The mesh is judged before it is committed: the panel shows the mesh
/// statistics and a depth overlay drawn over the live frame, so an operator
/// can check the recovered silhouettes against the image before a stencil is
/// created. See docs/reference/scene-lighting.md for the measurements behind
/// the approach.
class AppStage_DepthMeshCapture : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_DepthMeshCapture(class IEditorWindow* ownerWindow);
	virtual ~AppStage_DepthMeshCapture();

	/// Camera supplying the video feed, the calibrated FOV the metric depth
	/// recovery needs, and the pose the stencil is placed at. Call before
	/// enter().
	void setSourceCamera(CameraComponentPtr cameraComponent);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	// Required: AppStage::onGui() only draws the modal dialog stack, so a stage
	// that does not override this renders no panels at all.
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void setMenuState(eDepthMeshCaptureMenuState newState);

	void runCapture();
	bool createStencilFromMesh();

	/// Looks for any of the project's ArUco markers in the captured frame and,
	/// if one is found, compares its solvePnP corner depths against the
	/// sampled model depths to produce a scale correction factor.
	/// outCornerSpread is the worst per-corner disagreement with that factor -
	/// a consistency check on the geometry, not just the scale.
	bool tryComputeMarkerScaleCorrection(float& outFactor, float& outCornerSpread);

	/// Draws the recovered depth as colored points over the video frame so the
	/// silhouette alignment can be judged visually.
	void renderDepthPreview();

	// GUI button events
	void onCaptureEvent();
	void onApplyEvent();
	void onRedoEvent();
	void onCancelEvent();

private:
	class GuiPanel_DepthMeshCapture* m_capturePanel= nullptr;

	CameraComponentPtr m_currentSceneCameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;
	class VideoFrameDistortionView* m_monoDistortionView= nullptr;

	// Owned here rather than by the object system: the model is over a
	// gigabyte, so it is loaded when the capture tool is opened and freed when
	// it closes.
	std::unique_ptr<MoGeInference> m_inference;
	MoGeInference::Result m_geometry;
	DepthMeshGenerator::Mesh m_mesh;
	DepthMeshGenerator::Stats m_meshStats;
	bool m_bHasResult= false;

	// Scale calibration state for the current capture
	eDepthScaleCorrectionSource m_scaleCorrectionSource= eDepthScaleCorrectionSource::none;
	float m_appliedScaleCorrection= 1.f;
	float m_markerCornerSpread= 0.f;

	MikanCameraPtr m_mkCamera;
};
