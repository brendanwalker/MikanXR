#pragma once

#include "imgui.h"

#include <memory>
#include <string>
#include <vector>

struct MkGuiStyleVarEntry
{
	ImGuiStyleVar var;
	bool isVec2 = false;
	float floatVal = 0.f;
	ImVec2 vec2Val = {0.f, 0.f};
};

struct MkGuiStyleColorEntry
{
	ImGuiCol col;
	ImVec4 value;
};

class MkGuiStyle
{
public:
	std::string name;
	std::string fontName;   // "" = no font, "normal_icon", "big_icon"
	std::vector<MkGuiStyleVarEntry> vars;
	std::vector<MkGuiStyleColorEntry> colors;
};

using MkGuiStylePtr = std::shared_ptr<MkGuiStyle>;
using MkGuiStyleConstPtr = std::shared_ptr<const MkGuiStyle>;
