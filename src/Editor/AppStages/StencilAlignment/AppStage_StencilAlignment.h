#pragma once

//-- includes -----
#include "AppStage.h"
#include "StencilAligner.h"
#include "ColliderQuery.h"
#include "Constants_StencilAlignment.h"
#include "ComponentFwd.h"
#include "MikanRendererFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

class GuiPanel_StencilAlignment;

//-- definitions -----
class AppStage_StencilAlignment : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_StencilAlignment(class IEditorWindow* ownerWindow);
	virtual ~AppStage_StencilAlignment();

	inline void setSourceCamera(CameraComponentPtr camera) { m_cameraComponent = camera; }
	inline void setTargetStencil(ModelStencilComponentPtr stencil) { m_targetStencilComponent= stencil; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void setupStencilAligner();
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
	class GuiPanel_StencilAlignment* m_calibrationPanel = nullptr;

	CameraComponentPtr m_cameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;

	StencilAligner* m_stencilAligner;
	class VideoFrameDistortionView* m_monoDistortionView;
	
	ModelStencilComponentPtr m_targetStencilComponent;
	glm::vec3 m_boundingSphereCenter;
	float m_boundingSphereRadius;

	ColliderRaycastHitResult m_hoverResult;

	MkScenePtr m_scene;
	MikanCameraPtr m_mkCamera;
	IMkFrameBufferPtr m_frameBuffer;
	IMkTriangulatedMeshPtr m_fullscreenRGBQuad;
};