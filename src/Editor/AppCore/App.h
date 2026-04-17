#pragma once

//-- includes -----
#include "AppStage.h"
#include "IMkWindow.h"
#include "IMkWindowManager.h"
#include "ObjectSystemConfigFwd.h"

#include <chrono>

#include <memory>
#include <vector>
#include <assert.h>
#include <stdint.h>

// Forward declarations
class EventBus;

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
	inline class IMkWindow* getCurrentlyRenderingWindow() const { return m_renderingWindow; }
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
		
		if (appWindow->startup())
		{
			// pop this window context this window added if it created one
			// and return back to the previous window context
			if (m_windowManager->getCurrentWindowContext() == appWindow)
			{
				m_windowManager->popCurrentWindowContext(appWindow);
			}

			m_appWindows.push_back(appWindow);

			return appWindow;
		}
		else
		{
			destroyAppWindow(appWindow);
		}

		return appWindow;
	}

	template<typename t_app_window>
	void destroyAppWindow(t_app_window* appWindow)
	{
		// If this window was the current window, pop it from the current window stack
		if (m_windowManager->getCurrentWindowContext() == appWindow)
		{
			m_windowManager->popCurrentWindowContext(appWindow);
		}

		// Tear down the window and graphics context it owns
		appWindow->shutdown();

		// Remove the window from the list of windows (should deallocate it)
		auto it= std::find(m_appWindows.begin(), m_appWindows.end(), appWindow);
		if (it != m_appWindows.end())
		{
			m_appWindows.erase(it);
		}

		// If this was the main window pointer, make sure to invalidate that pointer
		if ((void *)appWindow == (void *)m_mainWindow)
		{
			m_mainWindow = nullptr;
		}

		delete appWindow;
	}

	template<typename t_app_window>
	bool hasWindowOfType() const
	{
		for (IMkWindow* window : m_appWindows)
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

private:
	static App* m_instance;

	// App Settings Config
	AppSettingsConfigPtr m_appSettings;

	// Global event bus for property changes and other events
	std::unique_ptr<EventBus> m_eventBus;

	// Localization manager
	class LocalizationManager* m_localizationManager= nullptr;

	// Window Manager
	IMkWindowManagerPtr m_windowManager;

	// Open windows (including the MainWindow)
	std::vector<IMkWindow*> m_appWindows;

	// The window being currently rendered
	IMkWindow* m_renderingWindow = nullptr;

	// The main window for the application
	class MainWindow* m_mainWindow= nullptr;

	// Flag requesting that we exit the update loop
	bool m_bShutdownRequested= false;

	// Current FPS rate
	std::chrono::steady_clock::time_point m_lastFrameTimestamp;
	float m_fps= 0.f;
};