#pragma once

#include "AppStage.h"
#include "Constants_LightFixtureCalibration.h"
#include "LightFixtureTriangulator.h"
#include "LightSystemFwd.h"
#include "MikanRendererFwd.h"
#include "MikanTypeFwd.h"
#include "MikanCoreTypes.h"
#include "VideoDisplayConstants.h"

#include <cstdint>
#include <memory>

class GuiPanel_LightFixtureCalibration;

class AppStage_LightFixtureCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_LightFixtureCalibration(class IEditorWindow* ownerWindow);
	virtual ~AppStage_LightFixtureCalibration();

	/// Set the target light fixture to calibrate. Call before enter().
	void setTargetFixture(DMXFixtureComponentPtr targetFixture);

	/// Set the camera to use for the video feed. Call before enter().
	void setSourceCamera(CameraComponentPtr cameraComponent);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	void setupDistortionView();
	void updateCameraTransform();
	void setMenuState(eLightFixtureCalibrationMenuState newState);

	// Input events
	void onMouseButtonUp(int button);

	// GUI button events
	void onOkEvent();
	void onRedoEvent();
	void onCancelEvent();

	// DMX helpers
	void flashFixtureWhite();
	void restoreFixtureColor();

private:
	class GuiPanel_LightFixtureCalibration* m_calibrationPanel= nullptr;

	CameraComponentPtr m_currentSceneCameraComponent;
	VideoSourceComponentPtr m_videoSourceComponent;

	LightFixtureTriangulator* m_triangulator= nullptr;
	class VideoFrameDistortionView* m_monoDistortionView= nullptr;

	DMXFixtureComponentPtr m_targetFixture;

	// Saved DMX color before we flash to white, for restoration on exit
	uint8_t m_savedRed= 0;
	uint8_t m_savedGreen= 0;
	uint8_t m_savedBlue= 0;
	bool m_colorSaved= false;

	MikanCameraPtr m_mkCamera;
};
