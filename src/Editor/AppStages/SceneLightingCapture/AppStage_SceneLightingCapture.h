#pragma once

#include "AppStage.h"
#include "Constants_SceneLightingCapture.h"
#include "LightSystemFwd.h"
#include "MikanCoreTypes.h"
#include "MikanRendererFwd.h"
#include "MikanTypeFwd.h"
#include "SceneLightingEstimator.h"
#include "VideoDisplayConstants.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

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
	/// Everything the estimate worker needs, prepared on the UI thread: the
	/// video buffers belong to it, and so does the tracked camera pose.
	struct EstimateRequest
	{
		cv::Mat bgrFrame;
		glm::mat3 cameraToWorldRotation= glm::mat3(1.f);
		float fovXDegrees= 0.f;
	};

	struct EstimateOutput
	{
		bool bSucceeded= false;
		bool bCancelled= false;
		std::string failureReason;
		std::string executionProvider;
		SceneLightingEstimator::Result result;
	};

	void setMenuState(eSceneLightingCaptureMenuState newState);

	void applyEstimate();

	// -- estimate worker --
	// Inference runs off the UI thread so the progress readout and the cancel
	// button stay live. The worker owns m_estimator for its whole lifetime
	// because OnnxSession must be created, run, and destroyed on one thread.
	void startEstimateWorker();
	void stopEstimateWorker();
	void estimateWorkerMain();
	void runEstimateRequest(const EstimateRequest& request, EstimateOutput& outOutput);
	/// Runs on the UI thread once the worker signals completion.
	void consumeEstimateOutput();

	/// Draws a sphere lit by the recovered environment so the estimate can be
	/// judged visually.
	void renderLitSpherePreview();

	// GUI button events
	void onCaptureEvent();
	void onCancelCaptureEvent();
	void onApplyEvent();
	void onRedoEvent();
	void onCancelEvent();

private:
	class GuiPanel_SceneLightingCapture* m_capturePanel= nullptr;

	CameraComponentPtr m_currentSceneCameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;
	class VideoFrameDistortionView* m_monoDistortionView= nullptr;

	LightEnvironmentComponentPtr m_targetProbe;

	// Owned by the worker thread rather than by the object system: the ONNX
	// models are several gigabytes, so they are loaded when the capture tool is
	// opened and freed when it closes - on the thread that created them. The UI
	// thread may only touch m_estimator under m_workerMutex, and only to call
	// the thread-safe requestCancel().
	std::thread m_workerThread;
	mutable std::mutex m_workerMutex;
	std::condition_variable m_workerSignal;
	bool m_bWorkerShutdownRequested= false;              ///< guarded by m_workerMutex
	bool m_bEstimateRequested= false;                    ///< guarded by m_workerMutex
	bool m_bEstimateFinished= false;                     ///< guarded by m_workerMutex
	EstimateRequest m_pendingRequest;                    ///< guarded by m_workerMutex
	EstimateOutput m_estimateOutput;                     ///< guarded by m_workerMutex
	std::unique_ptr<SceneLightingEstimator> m_estimator; ///< guarded by m_workerMutex

	std::atomic<int> m_estimatePhase{(int)eSceneLightingEstimatePhase::idle};
	std::atomic<int> m_estimateUnitsCompleted{0};
	std::atomic<int> m_estimateUnitsTotal{0};
	std::atomic<bool> m_bCancelRequested{false};
	float m_estimateElapsedSeconds= 0.f; ///< UI thread only

	SceneLightingEstimator::Result m_result;
	bool m_bHasResult= false;

	MikanCameraPtr m_mkCamera;
};
