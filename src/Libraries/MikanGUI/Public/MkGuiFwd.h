#pragma once

#include <memory>

typedef struct SDL_Window SDL_Window;

// ImGui/ImNodes opaque pointer forward declarations
struct ImFont;
struct ImGuiContext;
struct ImNodesContext;

// Attribute type for MkNodesScopedAttribute
enum class MkNodesAttributeType
{
	Input,
	Output,
	Static
};

class MkGuiContext;
using MkGuiContextPtr= std::shared_ptr<MkGuiContext>;
using MkGuiContextConstPtr= std::shared_ptr<const MkGuiContext>;