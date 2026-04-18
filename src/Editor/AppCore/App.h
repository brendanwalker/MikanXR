#pragma once

//-- includes -----
#include "AppStage.h"
#include "IMkWindow.h"
#include "IMkWindowManager.h"
#include "IEditorWindow.h"
#include "ObjectSystemConfigFwd.h"

#include <chrono>

#include <memory>
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
	inline IMkWindowManagerPtr getWindowManager() const { return m_windowManager; }
	IEditorWindow* getCurrentlyRenderingWindow() const;
	inline EventBus* getEventBus() const { return m_eventBus.get(); }
	inline class LocalizationManager* getLocalizationManager() const { return m_localizationManager; }

	inline float getFPS() const { return m_fps; }

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

protected:
	bool startup(int argc, char** argv);
	void shutdown();

	void tick();
	void tickWindows(const float deltaSeconds);

	bool createAppWindowInternal(EditorWindow* appWindow);

private:
	static App* m_instance;

	// App Settings Config
	AppSettingsConfigPtr m_appSettings;

	// Global event bus for property changes and other events
	std::unique_ptr<EventBus> m_eventBus;

	// Localization manager
	LocalizationManager* m_localizationManager= nullptr;

	// Window Context Manager
	IMkWindowManagerPtr m_windowManager;

	// Open windows (including the MainWindow)
	std::vector<EditorWindow*> m_appWindows;

	// The window being currently rendered
	EditorWindow* m_renderingWindow = nullptr;

	// The main window for the application
	MainWindow* m_mainWindow= nullptr;

	// Flag requesting that we exit the update loop
	bool m_bShutdownRequested= false;

	// Current FPS rate
	std::chrono::steady_clock::time_point m_lastFrameTimestamp;
	float m_fps= 0.f;
};