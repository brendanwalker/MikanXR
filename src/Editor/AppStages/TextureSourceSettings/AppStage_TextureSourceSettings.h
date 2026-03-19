#pragma once

//-- includes -----
#include "AppStage.h"
#include "ComponentFwd.h"
#include "MkRendererFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"

#include <memory>
#include <vector>

//-- definitions -----
class AppStage_TextureSourceSettings : public AppStage
{
public:
	AppStage_TextureSourceSettings(class IEditorWindow* ownerWindow);

	inline void setSourceCameraId(MikanCameraID cameraId)
	{
		m_cameraId= cameraId;
	}

	inline void setTextureSourceComponent(TextureSourceComponentPtr textureSourceComponent)
	{
		m_textureSourceComponent = textureSourceComponent;
	}

	virtual void enter() override;
	virtual void update(float deltaSeconds) override;
	virtual void render(IMkViewportPtr targetViewport) override;
	virtual void exit() override;

	static const char* APP_STAGE_NAME;

protected:
	// UI Events
	void onReturnEvent();

	// Remote Control
	bool handleRemoteControlCommand(
		const std::string& command,
		const std::vector<std::string>& parameters,
		std::vector<std::string>& outResults);
	bool handleGetTextureSourceComponentId(std::vector<std::string>& outResults);
	bool handleReturnRequest(std::vector<std::string>& outResults);

	class RmlModel_ClientTextureSourceComponent* m_clientTextureSourceComponentModel = nullptr;
	Rml::ElementDocument* m_TextureSourceSettingsView = nullptr;

	MikanCameraID m_cameraId= INVALID_MIKAN_ID;
	CameraComponentWeakPtr m_cameraComponent;
	TextureSourceComponentWeakPtr m_textureSourceComponent;
	IMkTriangulatedMeshPtr m_fullscreenRGBQuad;
	IMkTriangulatedMeshPtr m_fullscreenRGBAQuad;
	IMkTriangulatedMeshPtr m_fullscreenDepthUnpackQuad;

	float m_newFrameTimer = 0.f;
	static constexpr float k_newFrameTimerDuration = 1.f / 30.f;
};