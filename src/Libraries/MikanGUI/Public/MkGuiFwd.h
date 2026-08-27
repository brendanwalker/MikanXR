#pragma once

#include <memory>

typedef struct SDL_Window SDL_Window;

// ImGui opaque pointer forward declarations
struct ImFont;
struct ImGuiContext;

class MkGuiContext;
using MkGuiContextPtr= std::shared_ptr<MkGuiContext>;
using MkGuiContextConstPtr= std::shared_ptr<const MkGuiContext>;