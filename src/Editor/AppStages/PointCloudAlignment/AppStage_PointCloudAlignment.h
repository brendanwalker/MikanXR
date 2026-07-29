#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_PointCloudAlignment.h"
#include "ModelPointCloudAligner.h" // IcpResult
#include "ComponentFwd.h"
#include "MikanRendererFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

#include "glm/ext/vector_float2.hpp"

class GuiPanel_PointCloudAlignment;
class NaturalFeatureCloudBuilder;
class ModelPointCloudAligner;

//-- definitions -----
class AppStage_PointCloudAlignment : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_PointCloudAlignment(class IEditorWindow* ownerWindow);
	virtual ~AppStage_PointCloudAlignment();

	inline void setSourceCamera(CameraComponentPtr camera) { m_cameraComponent= camera; }
	inline void setTargetStencil(ModelStencilComponentPtr stencil) { m_targetStencilComponent= stencil; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void setupTools();
	void updateXRCamera();
	void updateVRCamera();
	void renderStencilScene();
	void setMenuState(ePointCloudAlignmentMenuState newState);
	void runAlignment();

	// Viewport Events
	void onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);

	// Calibration Model UI Events
	void onOkEvent();
	void onRedoEvent();
	void onCancelEvent();
	void onBeginRoiEvent();
	void onSkipRoiEvent();
	void onStartCaptureEvent();
	void onStopCaptureEvent();
	void onRunAlignmentEvent();

private:
	class GuiPanel_PointCloudAlignment* m_calibrationPanel= nullptr;

	CameraComponentPtr m_cameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;

	NaturalFeatureCloudBuilder* m_cloudBuilder= nullptr;
	ModelPointCloudAligner* m_aligner= nullptr;
	class VideoFrameDistortionView* m_monoDistortionView= nullptr;

	ModelStencilComponentPtr m_targetStencilComponent;
	glm::vec3 m_boundingSphereCenter;
	float m_boundingSphereRadius= 1.f;

	// Coarse pose that competes as an extra alignment hypothesis (the stencil's placement on entry)
	glm::mat4 m_initialGuess= glm::mat4(1.f);
	IcpResult m_lastIcpResult;
	bool m_alignmentPending= false;

	// Optional region-of-interest capture (two clicks in the video)
	int m_roiClickCount= 0;
	glm::vec2 m_roiCorners[2]= {glm::vec2(0.f), glm::vec2(0.f)};

	MkScenePtr m_scene;
	MikanCameraPtr m_mkCamera;
	IMkFrameBufferPtr m_frameBuffer;
	IMkTriangulatedMeshPtr m_fullscreenRGBQuad;
};
