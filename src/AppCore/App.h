#pragma once

//-- includes -----
#include "AppStage.h"
#include "SDL_events.h"
#include <vector>
#include <assert.h>
#include <stdint.h>

//-- definitions -----
class App
{
public:
	App();
	virtual ~App();

	static App* getInstance() { return m_instance; }

	inline class ProfileConfig* getProfileConfig() const { return m_profileConfig; }
	inline class MikanServer* getMikanServer() const { return m_mikanServer; }
	inline class Renderer* getRenderer() const { return m_renderer; }
	inline class GlShaderCache* getShaderCache() const { return m_shaderCache; }
	inline class GlBakedTextCache* getBakedTextCache() const { return m_bakedTextCache; }
	inline class VideoSourceManager* getVideoSourceManager() const { return m_videoSourceManager; }
	inline class VRDeviceManager* getVRDeviceManager() const { return m_vrDeviceManager; }

	inline float getFPS() const { return m_fps; }

	int exec(int argc, char** argv);

	inline void requestShutdown()
	{
		m_bShutdownRequested = true;
	}

	inline AppStage* getCurrentAppStage() const
	{
		return (m_appStageStack.size() > 0) ? m_appStageStack[m_appStageStack.size() - 1] : nullptr;
	}

	inline AppStage* getParentAppStage() const
	{
		return (m_appStageStack.size() > 1) ? m_appStageStack[m_appStageStack.size() - 2] : nullptr;
	}

	template<typename t_app_stage>
	t_app_stage* pushAppStage()
	{
		assert(bAppStackOperationAllowed);
		t_app_stage* appStage = new t_app_stage(this);
		AppStage* parentAppStage=
			m_appStageStack.size() > 0
			? m_appStageStack[m_appStageStack.size() - 1]
			: nullptr;

		m_appStageStack.push_back(appStage);
		m_pendingAppStageOps.push_back({ parentAppStage, appStage, AppStageOperation::enter});

		return appStage;
	}

	void popAppState()
	{
		assert(bAppStackOperationAllowed);
		AppStage* appStage = getCurrentAppStage();
		if (appStage != nullptr)
		{
			m_appStageStack.pop_back();

			AppStage* parentAppStage =
				m_appStageStack.size() > 0
				? m_appStageStack[m_appStageStack.size() - 1]
				: nullptr;

			m_pendingAppStageOps.push_back({ parentAppStage, appStage, AppStageOperation::exit });
		}
	}

protected:
	bool startup(int argc, char** argv);
	void shutdown();

	void onSDLEvent(SDL_Event& e);

	void update();
	void render();

private:
	static App* m_instance;

	// Profile Config
	class ProfileConfig* m_profileConfig= nullptr;

	// Mikan API Server
	class MikanServer* m_mikanServer= nullptr;

	// Used to blend video with client render targets
	class GlFrameCompositor* m_frameCompositor= nullptr;

	// Input Manager
	class InputManager* m_inputManager= nullptr;

	// Localization manager
	class LocalizationManager* m_localizationManager= nullptr;

	// OpenGL renderer
	class Renderer* m_renderer= nullptr;

	// OpenGL shader program chache
	class GlShaderCache *m_shaderCache = nullptr;

	// OpenGL/SDL font/baked text string texture cache
	class GlBakedTextCache* m_bakedTextCache = nullptr;

	// Keeps track of currently connected camera
	class VideoSourceManager* m_videoSourceManager = nullptr;

	// Keeps track of currently connected VR trackers
	class VRDeviceManager* m_vrDeviceManager = nullptr;

	// App Stages
	int m_appStageStackIndex= -1;
	std::vector<AppStage*> m_appStageStack;

	enum class AppStageOperation : int
	{
		enter,
		exit
	};

	struct PendingAppStageOperation
	{
		AppStage* parentAppStage;
		AppStage* appStage;
		AppStageOperation op;
	};
	std::vector<PendingAppStageOperation> m_pendingAppStageOps;
	bool bAppStackOperationAllowed = true;

	// Flag requesting that we exit the update loop
	bool m_bShutdownRequested= false;

	// Current FPS rate
	uint32_t m_lastFrameTimestamp= 0;
	float m_fps= 0.f;
};