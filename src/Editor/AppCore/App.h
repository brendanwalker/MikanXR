#pragma once

//-- includes -----
#include "AppStage.h"
#include "IGlWindow.h"
#include "ObjectSystemConfigFwd.h"

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
	inline class MainWindow* getMainWindow() const { return m_mainWindow; }
	inline class SdlManager* getSdlManager() const { return m_sdlManager; }
	inline class IGlWindow* getCurrentlyRenderingWindow() const { return m_renderingWindow; }

	inline float getFPS() const { return m_fps; }

	int exec(int argc, char** argv);

	inline void requestShutdown()
	{
		m_bShutdownRequested = true;
	}

	template<typename t_app_window>
	t_app_window* createAppWindow()
	{
		t_app_window* appWindow= new t_app_window();
		
		if (appWindow->startup())
		{
			// pop this GL Context this window added if it created one 
			// and return back to the previous GL Context
			if (getCurrentGlContext() == appWindow)
			{
				popCurrentGlContext(appWindow);
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
		if (m_glContextStack.size() > 0 && m_glContextStack.back() == appWindow)
		{
			popCurrentGlContext(appWindow);
		}

		// Tear down the SDL window and OpenGL context
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
		for (IGlWindow* window : m_appWindows)
		{
			if (dynamic_cast<t_app_window*>(window) != nullptr)
			{
				return true;
			}
		}

		return false;
	}

	void pushCurrentGLContext(class IGlWindow* window);
	class IGlWindow* getCurrentGlContext() const;
	void popCurrentGlContext(class IGlWindow* window);

protected:
	bool startup(int argc, char** argv);
	void shutdown();

	void tick();
	void tickWindows(const float deltaSeconds);
	void updateAutoSave(float deltaSeconds);

private:
	static App* m_instance;

	// Profile Config
	ProfileConfigPtr m_profileConfig;
	float m_profileSaveCooldown= -1.f;

	// Localization manager
	class LocalizationManager* m_localizationManager= nullptr;

	// SDL Top Level Management
	class SdlManager* m_sdlManager;

	// Open windows (including the MainWindow)
	std::vector<IGlWindow*> m_appWindows;

	// The stack of current windows being updated
	std::vector<IGlWindow*> m_glContextStack;

	// The window being currently rendered
	IGlWindow* m_renderingWindow = nullptr;

	// The main window for the application
	class MainWindow* m_mainWindow= nullptr;

	// Flag requesting that we exit the update loop
	bool m_bShutdownRequested= false;

	// Current FPS rate
	uint32_t m_lastFrameTimestamp= 0;
	float m_fps= 0.f;
};