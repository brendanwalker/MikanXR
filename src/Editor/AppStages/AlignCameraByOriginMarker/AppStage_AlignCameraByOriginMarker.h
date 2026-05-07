#pragma once

//-- includes -----
#include "AppStage.h"
#include "ComponentFwd.h"
#include "Constants_AlignCameraByOriginMarker.h"
#include "MikanCoreTypes.h"
#include "MikanRendererFwd.h"
#include "MikanTypeFwd.h"

#include "glm/ext/matrix_double4x4.hpp"

#include <memory>

class GuiPanel_AlignCameraByOriginMarker;

//-- definitions -----
class AppStage_AlignCameraByOriginMarker : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_AlignCameraByOriginMarker(class IEditorWindow* ownerWindow);
	virtual ~AppStage_AlignCameraByOriginMarker();

	static bool tryEnterCalibration(
		AppStage* fromAppStage,
		CameraComponentPtr targetCameraComponent);

	void setTargetCameraComponent(CameraComponentPtr cameraComponent);

	// -- AppStage -- //
	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void startVideoStream();
	void setupCalibrator();
	void updateVerifySetup();
	void updateCapturing();
	void computeAndApplyTargetTransform();
	void setMenuState(eAlignCameraByOriginMarkerMenuState newState);

	void onBeginEvent();
	void onRestartEvent();
	void onCancelEvent();
	void onReturnEvent();

private:
	GuiPanel_AlignCameraByOriginMarker* m_calibrationPanel = nullptr;

	// Target camera (being calibrated - no tracking mount)
	CameraComponentPtr                   m_targetCameraComponent;
	VideoSourceComponentPtr              m_targetVideoSource;
	class VideoFrameDistortionView*      m_targetDistortionView = nullptr;
	class ArucoMarkerPoseSampler*        m_targetMarkerSampler  = nullptr;

	// Origin marker ID from tracking volume
	MikanMarkerID m_originMarkerId = INVALID_MIKAN_ID;

	// Final computed camera aperture pose in stage space
	glm::dmat4 m_cameraApertureXform_StageSpace;

	IMkFrameBufferPtr      m_frameBuffer;
	IMkTriangulatedMeshPtr m_fullscreenQuad;
};
