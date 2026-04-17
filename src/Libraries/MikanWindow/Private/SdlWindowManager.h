#pragma once

//-- includes -----
#include "IMkWindowManager.h"
#include "SdlFwd.h"

#include <string>
#include <vector>

//-- definitions -----
class SdlWindowManager : public IMkWindowManager
{
public:
	SdlWindowManager();
	~SdlWindowManager();

	// -- IMkWindowManager --
	virtual bool startup() override;
	virtual void shutdown() override;
	virtual void pollEvents() override;

	virtual bool getIsInitialized() const override { return m_sdlInitialized; }
	virtual const std::string& getGlslVersion() const override { return m_glslVersion; }
	virtual void setMouseCursor(const std::string& cursorName) override;

	virtual void pushCurrentWindowContext(IMkWindow* window) override;
	virtual IMkWindow* getCurrentWindowContext() const override;
	virtual void popCurrentWindowContext(IMkWindow* window) override;

	std::vector<SDL_Event>& getEvents() { return m_events; }

private:
	bool m_sdlInitialized= false;
	std::string m_glslVersion;

	std::vector<IMkWindow*> m_mkWindowContextStack;

	SDL_Cursor* cursor_default = nullptr;
	SDL_Cursor* cursor_move = nullptr;
	SDL_Cursor* cursor_pointer = nullptr;
	SDL_Cursor* cursor_resize = nullptr;
	SDL_Cursor* cursor_cross = nullptr;
	SDL_Cursor* cursor_text = nullptr;
	SDL_Cursor* cursor_unavailable = nullptr;

	std::vector<SDL_Event> m_events;
};
