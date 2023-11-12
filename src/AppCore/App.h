#pragma once

//-- includes -----
#include "AppStage.h"
#include "IGlWindow.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "SDL_events.h"

#include <memory>
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

	inline ProfileConfigPtr getProfileConfig() const { return m_profileConfig; }
	inline class MikanServer* getMikanServer() const { return m_mikanServer; }
	inline ObjectSystemManagerPtr getObjectSystemManager() const { return m_objectSystemManager; }
	inline class MainWindow* getMainWindow() const { return m_mainWindow; }
	inline class FontManager* getFontManager() const { return m_fontManager; }
	inline class VideoSourceManager* getVideoSourceManager() const { return m_videoSourceManager; }
	inline class VRDeviceManager* getVRDeviceManager() const { return m_vrDeviceManager; }
	inline class RmlManager* getRmlManager() const { return m_rmlManager; }
	inline class SdlManager* getSdlManager() const { return m_sdlManager.get(); }
	inline class GlFrameCompositor* getFrameCompositor() const { return m_frameCompositor; }
	inline class IGlWindow* getCurrentlyRenderingWindow() const { return m_renderingWindow; }

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

	template<typename t_app_window>
	t_app_window* createAppWindow()
	{
		t_app_window* appWindow= new t_app_window();

		if (appWindow->startup())
		{
			m_appWindows.push_back(appWindow);

			return appWindow;
		}
		else
		{
			appWindow->shutdown();

			delete appWindow;
			appWindow= nullptr;
		}

		return appWindow;
	}

	template<typename t_app_window>
	void destroyAppWindow(t_app_window* appWindow)
	{
		auto it= std::find(m_appWindows.begin(), m_appWindows.end(), appWindow);
		if (it != m_appWindows.end())
		{
			m_appWindows.erase(it);
		}

		appWindow->shutdown();
		delete appWindow;
	}

	MulticastDelegate<void(AppStage* appStage)> OnAppStageEntered;
	MulticastDelegate<void(AppStage* appStage)> OnAppStageExited;

protected:
	bool startup(int argc, char** argv);
	void shutdown();

	void onSDLEvent(SDL_Event& e);

	void update();
	void processPendingAppStageOps();
	void updateAutoSave(float deltaSeconds);
	void render();

private:
	static App* m_instance;

	// Profile Config
	ProfileConfigPtr m_profileConfig;
	float m_profileSaveCooldown= -1.f;

	// Mikan API Server
	class MikanServer* m_mikanServer= nullptr;

	// Used to blend video with client render targets
	class GlFrameCompositor* m_frameCompositor= nullptr;

	// Input Manager
	class InputManager* m_inputManager= nullptr;

	// Rml UI Manager
	class RmlManager* m_rmlManager= nullptr;

	// Localization manager
	class LocalizationManager* m_localizationManager= nullptr;

	// Object System manager
	ObjectSystemManagerPtr m_objectSystemManager;

	// OpenCV management
	std::unique_ptr<class OpenCVManager> m_openCVManager;

	// SDL Top Level Management
	std::unique_ptr<class SdlManager> m_sdlManager;

	// OpenGL/SDL font/baked text string texture cache
	class FontManager* m_fontManager = nullptr;

	// Keeps track of currently connected camera
	class VideoSourceManager* m_videoSourceManager = nullptr;

	// Keeps track of currently connected VR trackers
	class VRDeviceManager* m_vrDeviceManager = nullptr;

	// Open windows (including the MainWindow)
	std::vector<IGlWindow*> m_appWindows;

	// The window being currently rendered
	IGlWindow* m_renderingWindow = nullptr;

	// The main window for the application
	class MainWindow* m_mainWindow= nullptr;

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