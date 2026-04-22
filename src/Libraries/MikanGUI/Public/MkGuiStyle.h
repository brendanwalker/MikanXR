#pragma once

#include "imgui.h"
#include "MkRendererFwd.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

struct MkGuiStyleFloatEntry
{
	ImGuiStyleVar var;
	float floatVal = 0.f;
};

struct MkGuiStyleVec2Entry
{
	ImGuiStyleVar var;
	ImVec2 vec2Val = { 0.f, 0.f };
};

struct MkGuiStyleColorEntry
{
	ImGuiCol col;
	ImVec4 value;
};

struct MkGuiStyleTextureEntry
{
	float x = 0.f;  // display width
	float y = 0.f;  // display height
	IMkTextureConstPtr texture;
};

class MkGuiStyle
{
public:
	static constexpr int k_defaultLabelWidth = 200;
	static constexpr int k_defaultValueWidth = 150;

	std::string name;
	int labelWidth= k_defaultLabelWidth;
	int valueWidth= k_defaultValueWidth;
	std::vector<MkGuiStyleFloatEntry> floatVars;
	std::vector<MkGuiStyleVec2Entry> vec2Vars;
	std::vector<MkGuiStyleColorEntry> colors;
	std::map<std::string, MkGuiStyleTextureEntry> textures;
	struct ImFont* font = nullptr;  // Optional named font to use for this style (e.g. "normal_icon", "big_icon")
};

using MkGuiStylePtr = std::shared_ptr<MkGuiStyle>;
using MkGuiStyleConstPtr = std::shared_ptr<const MkGuiStyle>;
