//-- includes -----
#include "SdlManager.h"
#include "Logger.h"
#include "Version.h"

#if defined(_WIN32)
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_syswm.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_syswm.h>
#endif

#include "GlCommon.h"

#include <algorithm>
#include <assert.h>

//-- statics -----
SdlManager* SdlManager::m_instance = NULL;

//-- public methods -----
SdlManager::SdlManager()
	: m_sdlInitialized(false)
{}

SdlManager::~SdlManager()
{
	assert(!m_sdlInitialized);
	assert(m_instance == nullptr);
}

bool SdlManager::startup()
{
	bool success = true;

	MIKAN_LOG_INFO("SdlManager::init()") << "Initializing SDL Library";
	m_instance = this;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) == 0)
	{
		m_sdlInitialized = true;
	}
	else
	{
		MIKAN_LOG_ERROR("SdlManager::init") << "Unable to initialize SDL: " << SDL_GetError();
		success = false;
	}

	if (success)
	{
		// Decide GL+GLSL versions
	#if defined(IMGUI_IMPL_OPENGL_ES2)
		// GL ES 2.0 + GLSL 100
		m_glslVersion = "#version 100";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	#elif defined(__APPLE__)
		// GL 3.2 Core + GLSL 150
		m_glslVersion = "#version 150";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	#else
		// GL 3.0 + GLSL 130
		m_glslVersion = "#version 130";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	#endif

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	}

	if (success)
	{
		cursor_default = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		cursor_move = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
		cursor_pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
		cursor_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
		cursor_cross = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
		cursor_text = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
		cursor_unavailable = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
	}

	return success;
}

void SdlManager::shutdown()
{
	// Free cursors
	if (cursor_default != nullptr)
	{
		SDL_FreeCursor(cursor_default);
		cursor_default = nullptr;
	}
	if (cursor_move != nullptr)
	{
		SDL_FreeCursor(cursor_move);
		cursor_move = nullptr;
	}
	if (cursor_pointer != nullptr)
	{
		SDL_FreeCursor(cursor_pointer);
		cursor_pointer = nullptr;
	}
	if (cursor_resize != nullptr)
	{
		SDL_FreeCursor(cursor_resize);
		cursor_resize = nullptr;
	}
	if (cursor_cross != nullptr)
	{
		SDL_FreeCursor(cursor_cross);
		cursor_cross = nullptr;
	}
	if (cursor_text != nullptr)
	{
		SDL_FreeCursor(cursor_text);
		cursor_text = nullptr;
	}
	if (cursor_unavailable != nullptr)
	{
		SDL_FreeCursor(cursor_unavailable);
		cursor_unavailable = nullptr;
	}

	if (m_sdlInitialized)
	{
		SDL_Quit();
		m_sdlInitialized = false;
	}

	m_instance = NULL;
}

void SdlManager::pollEvents()
{
	m_events.clear();

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		m_events.push_back(event);
	}
}

void SdlManager::setSDLMouseCursor(const std::string& cursor_name)
{
	SDL_Cursor* cursor = nullptr;

	if (cursor_name.empty() || cursor_name == "arrow")
		cursor = cursor_default;
	else if (cursor_name == "move")
		cursor = cursor_move;
	else if (cursor_name == "pointer")
		cursor = cursor_pointer;
	else if (cursor_name == "resize")
		cursor = cursor_resize;
	else if (cursor_name == "cross")
		cursor = cursor_cross;
	else if (cursor_name == "text")
		cursor = cursor_text;
	else if (cursor_name == "unavailable")
		cursor = cursor_unavailable;

	if (cursor)
		SDL_SetCursor(cursor);
}