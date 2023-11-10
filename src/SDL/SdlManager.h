#pragma once

//-- includes -----
#include "SdlFwd.h"

#include <string>

//-- definitions -----
class SdlManager
{
public:
	SdlManager();
	~SdlManager();

	bool startup();
	void shutdown();

	void setSDLMouseCursor(const std::string& cursor_name);

	static SdlManager* getInstance()
	{
		return m_instance;
	}

private:
	bool m_sdlInitialized;

	struct SDL_Cursor* cursor_default = nullptr;
	struct SDL_Cursor* cursor_move = nullptr;
	struct SDL_Cursor* cursor_pointer = nullptr;
	struct SDL_Cursor* cursor_resize = nullptr;
	struct SDL_Cursor* cursor_cross = nullptr;
	struct SDL_Cursor* cursor_text = nullptr;
	struct SDL_Cursor* cursor_unavailable = nullptr;

	static SdlManager* m_instance;
};
