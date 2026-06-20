#pragma once

#include "imgui.h"
#include "MkRendererFwd.h"

#include <memory>

struct MkGuiStyleFloatEntry
{
	ImGuiStyleVar var;
	float floatVal= 0.f;
};

struct MkGuiStyleVec2Entry
{
	ImGuiStyleVar var;
	ImVec2 vec2Val= {0.f, 0.f};
};

struct MkGuiStyleColorEntry
{
	ImGuiCol col;
	ImVec4 value;
};

struct MkGuiStyleTextureEntry
{
	float x= 0.f; // display width
	float y= 0.f; // display height
	IMkTextureConstPtr texture;
};
