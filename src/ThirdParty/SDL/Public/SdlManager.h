#pragma once

//-- includes -----
#include "SdlFwd.h"

#include <string>
#include <vector>

//-- definitions -----
class SdlManager
{
public:
	SdlManager();
	~SdlManager();

	bool startup();
	void shutdown();

	void pollEvents();
	std::vector<SDL_Event>& getEvents() { return m_events; }

	void setSDLMouseCursor(const std::string& cursor_name);

	bool getIsSdlInitialized() const { return m_sdlInitialized; }
	const std::string& getGlslVersion() const { return m_glslVersion; }

	static SdlManager* getInstance() { return m_instance; }

private:
	bool m_sdlInitialized= false;
	std::string m_glslVersion;

	SDL_Cursor* cursor_default = nullptr;
	SDL_Cursor* cursor_move = nullptr;
	SDL_Cursor* cursor_pointer = nullptr;
	SDL_Cursor* cursor_resize = nullptr;
	SDL_Cursor* cursor_cross = nullptr;
	SDL_Cursor* cursor_text = nullptr;
	SDL_Cursor* cursor_unavailable = nullptr;

	std::vector<SDL_Event> m_events;

	static SdlManager* m_instance;
};
