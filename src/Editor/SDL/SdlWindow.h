#pragma once

//-- includes -----
#include "SdlFwd.h"

#include <memory>
#include <string>

//-- definitions -----
class SdlWindow 
{
public:
	SdlWindow()= default;
	SdlWindow(class ISdlMkWindow* ownerWindowInterface);

	bool startup();
	void shutdown();

	void focus();
	void handleSDLEvents();

	void renderBegin();
	void renderEnd();

	void makeGlContextCurrent();

	int getWindowId() const { return m_windowId; }

	// NOTE: This needs to be called before startup for it to do anything
	SdlWindow* enableGLDataSharing();
	bool isGlDataSharingEnabled() const { return m_bGLDataSharingEnabled; }

	SdlWindow* setTitle(const std::string& title);
	const std::string& getTitle() const { return m_title; }

	SdlWindow* setSize(int width, int height);
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	float getAspectRatio() const { return (float)m_width / (float)m_height; }

	bool hasMouseFocus() const { return m_hasMouseFocus; }
	bool hasKeyboardFocus() const { return m_hasKeyboardFocus; }
	bool isMinimized() const { return m_isMinimized; }
	bool isShown() const { return m_isShown; }
	bool wantsDestroy() const { return m_wantsDestroy; }

	inline SDL_Window* getInternalSdlWindow() { return m_sdlWindow; }
	inline void* getInternalGlContext() { return m_glContext; }

protected:
	bool handleSDLWindowEvent(const SDL_Event* event);

private:
	class ISdlMkWindow* m_owner= nullptr;
	SDL_Window* m_sdlWindow= nullptr;
	void* m_glContext = nullptr;
	int m_windowId= -1;

	// Settings
	bool m_bGLDataSharingEnabled= false;

	// Titlebar
	std::string m_title;

	// Window dimensions
	int m_width= 0;
	int m_height= 0;

	//Window focus
	bool m_hasMouseFocus= false;
	bool m_hasKeyboardFocus= false;
	bool m_isfullScreen= false;
	bool m_isMinimized= false;
	bool m_isShown= false;
	bool m_wantsDestroy= false;
};