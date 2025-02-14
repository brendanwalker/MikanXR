#pragma once

#include <memory>

typedef union SDL_Event SDL_Event;
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Cursor SDL_Cursor;

class SdlWindow;
using SdlWindowUniquePtr = std::unique_ptr<SdlWindow>;