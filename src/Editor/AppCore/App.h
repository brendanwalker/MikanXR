#pragma once

//-- includes -----
#include "AppStage.h"
#include "IMkWindowContext.h"
#include "IMkWindowContextManager.h"
#include "IEditorWindow.h"
#include "ObjectSystemConfigFwd.h"

#include <chrono>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <assert.h>
#include <stdint.h>

// Forward declarations
class EditorWindow;
class EventBus;
class LocalizationManager;
class MainWindow;

//-- definitions -----
class App 
{
public:
	App();
	virtual ~App();

	static App* getInstance() { return m_instance; }

	inline AppSettingsConfigPtr getAppSettings() const { return m_appSettings; }
	inline class MainWindow* getMainWindow() const { return m_mainWindow; }
	inline IMkWindowContextManagerPtr getWindowManager() const { return m_windowManager; }
	IEditorWindow* getCurrentlyRenderingWindow() const;
	inline EventBus* getEventBus() const { return m_eventBus.get(); }
	inline class LocalizationManager* getLocalizationManager() const { return m_localizationManager; }

	inline double getSecondsSinceAppStart() const { return m_secondsSinceAppStart; }
	inline float getFPS() const { return m_fps; }

	bool hasCommandLineFlag(const std::string& flag) const;
	std::string getCommandLineStringArg(const std::string& key, const std::string& defaultValue = "") const;

	int exec(int argc, char** argv);

	inline void requestShutdown()
	{
		m_bShutdownRequested = true;
	}

	template<typename t_app_window>
	t_app_window* createAppWindow()
	{
		t_app_window* appWindow= new t_app_window(this);
		
		// Setup the window
		// Destroy the window if setup fails
		if (!createAppWindowInternal(appWindow))
		{
			return nullptr;
		}

		return appWindow;
	}

	void destroyAppWindow(EditorWindow* appWindow);

	template<typename t_app_window>
	bool hasWindowOfType() const
	{
		for (EditorWindow* window : m_appWindows)
		{
			if (dynamic_cast<t_app_window*>(window) != nullptr)
			{
				return true;
			}
		}

		return false;
	}

	template<typename t_app_window>
	t_app_window* getWindowOfType() const
	{
		for (EditorWindow* window : m_appWindows)
		{
			t_app_window* typedWindow = dynamic_cast<t_app_window*>(window);
			if (typedWindow != nullptr)
				return typedWindow;
		}

		return nullptr;
	}

protected:
	bool startup(int argc, char** argv);
	void shutdown();

	void tick();
	void tickWindows(const float deltaSeconds);

	bool createAppWindowInternal(EditorWindow* appWindow);

	bool runTests();

private:
	static App* m_instance;

	// Test mode state (set when -run-tests flag is present)
	bool m_testMode = false;
	bool m_testsPassed = false;

	// App Settings Config
	AppSettingsConfigPtr m_appSettings;

	// Global event bus for property changes and other events
	std::unique_ptr<EventBus> m_eventBus;

	// Localization manager
	LocalizationManager* m_localizationManager= nullptr;

	// Window Context Manager
	IMkWindowContextManagerPtr m_windowManager;

	// Open windows (including the MainWindow)
	std::vector<EditorWindow*> m_appWindows;

	// The window being currently rendered
	EditorWindow* m_renderingWindow = nullptr;

	// The main window for the application
	MainWindow* m_mainWindow= nullptr;

	// Flag requesting that we exit the update loop
	bool m_bShutdownRequested= false;

	// Command line arguments parsed at startup
	std::map<std::string, std::string> m_commandLineParams;
	std::set<std::string> m_commandLineFlags;

	// Current FPS rate
	double m_secondsSinceAppStart = 0.0;
	std::chrono::steady_clock::time_point m_lastFrameTimestamp;
	float m_fps= 0.f;
};