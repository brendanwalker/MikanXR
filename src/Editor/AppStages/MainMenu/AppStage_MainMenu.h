#pragma once

//-- includes -----
#include "AppStage.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"

//-- definitions -----
class AppStage_MainMenu : public AppStage
{
public:
	AppStage_MainMenu(class IEditorWindow* ownerWindow);

	virtual void enter() override;

	static const char* APP_STAGE_NAME;

protected:
	void onResumeProject();
	void onOpenProject();
	void onNewProject();
	void onExit();

	// -- AppStage --
	virtual void update(float deltaSeconds);
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport);

	// -- IRemoteControllableAppStage Interface -- //
	virtual bool handleRemoteControlCommand(
		const std::string& command,
		const std::vector<std::string>& parameters,
		std::vector<std::string>& outResults);
	bool handleResumeProjectCommand(std::vector<std::string>& outResults);
	bool handleOpenProjectCommand(const std::vector<std::string>& parameters, std::vector<std::string>& outResults);
	bool handleNewProjectCommand(const std::vector<std::string>& parameters, std::vector<std::string>& outResults);
	bool handleExitCommand(std::vector<std::string>& outResults);

private:
	AppSettingsConfigPtr m_appSettingsConfig;
	ProjectManagerPtr m_projectManager;
	IMkTexturePtr m_backgroundTexture;
	IMkTriangulatedMeshPtr m_fullscreenRGBQuad;
	float m_shaderTime = 0.f;
};
