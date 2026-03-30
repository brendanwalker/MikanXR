#pragma once

#include <memory>

typedef struct SDL_Window SDL_Window;

class MkGuiContext;
using MkGuiContextPtr = std::shared_ptr<MkGuiContext>;
using MkGuiContextConstPtr = std::shared_ptr<const MkGuiContext>;