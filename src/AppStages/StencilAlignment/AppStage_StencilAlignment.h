#pragma once

//-- includes -----
#include "AppStage.h"
#include "StencilAligner.h"
#include "ColliderQuery.h"
#include "Constants_StencilAlignment.h"
#include "ComponentFwd.h"
#include "RendererFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

//-- definitions -----
class AppStage_StencilAlignment : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_StencilAlignment(class MainWindow* ownerWindow);
	virtual ~AppStage_StencilAlignment();

	inline void setTargetStencil(ModelStencilComponentPtr stencil) { m_targetStencilComponent= stencil; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void updateXRCamera();
	void updateVRCamera();
	void renderStencilScene();
	void setMenuState(eStencilAlignmentMenuState newState);

	// Viewport Events
	void onMouseRayChanged(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);

	// Calibration Model UI Events
	void onOkEvent();
	void onRedoEvent();
	void onCancelEvent();

private:
	class RmlModel_StencilAlignment* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	VideoSourceViewPtr m_videoSourceView;

	// Tracking puck used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	StencilAligner* m_stencilAligner;
	class VideoFrameDistortionView* m_monoDistortionView;
	
	ModelStencilComponentPtr m_targetStencilComponent;

	ColliderRaycastHitResult m_hoverResult;

	GlScenePtr m_scene;
	GlCameraPtr m_camera;
	GlFrameBufferPtr m_frameBuffer;
	GlTriangulatedMeshPtr m_fullscreenQuad;
};