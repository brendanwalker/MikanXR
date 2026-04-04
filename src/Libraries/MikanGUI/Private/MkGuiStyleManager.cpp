#include "MkGuiStyleManager.h"
#include "MkGuiContext.h"
#include "Logger.h"

#include "nlohmann/json.hpp"

#include <fstream>

// Maps JSON var name to {ImGuiStyleVar enum, isVec2}
static const std::unordered_map<std::string, std::pair<ImGuiStyleVar, bool>> k_styleVarTable = {
	{"Alpha",                {ImGuiStyleVar_Alpha,                false}},
	{"DisabledAlpha",        {ImGuiStyleVar_DisabledAlpha,        false}},
	{"WindowPadding",        {ImGuiStyleVar_WindowPadding,        true }},
	{"WindowRounding",       {ImGuiStyleVar_WindowRounding,       false}},
	{"WindowBorderSize",     {ImGuiStyleVar_WindowBorderSize,     false}},
	{"WindowMinSize",        {ImGuiStyleVar_WindowMinSize,        true }},
	{"WindowTitleAlign",     {ImGuiStyleVar_WindowTitleAlign,     true }},
	{"ChildRounding",        {ImGuiStyleVar_ChildRounding,        false}},
	{"ChildBorderSize",      {ImGuiStyleVar_ChildBorderSize,      false}},
	{"PopupRounding",        {ImGuiStyleVar_PopupRounding,        false}},
	{"PopupBorderSize",      {ImGuiStyleVar_PopupBorderSize,      false}},
	{"FramePadding",         {ImGuiStyleVar_FramePadding,         true }},
	{"FrameRounding",        {ImGuiStyleVar_FrameRounding,        false}},
	{"FrameBorderSize",      {ImGuiStyleVar_FrameBorderSize,      false}},
	{"ItemSpacing",          {ImGuiStyleVar_ItemSpacing,          true }},
	{"ItemInnerSpacing",     {ImGuiStyleVar_ItemInnerSpacing,     true }},
	{"IndentSpacing",        {ImGuiStyleVar_IndentSpacing,        false}},
	{"CellPadding",          {ImGuiStyleVar_CellPadding,          true }},
	{"ScrollbarSize",        {ImGuiStyleVar_ScrollbarSize,        false}},
	{"ScrollbarRounding",    {ImGuiStyleVar_ScrollbarRounding,    false}},
	{"GrabMinSize",          {ImGuiStyleVar_GrabMinSize,          false}},
	{"GrabRounding",         {ImGuiStyleVar_GrabRounding,         false}},
	{"TabRounding",          {ImGuiStyleVar_TabRounding,          false}},
	{"ButtonTextAlign",      {ImGuiStyleVar_ButtonTextAlign,      true }},
	{"SelectableTextAlign",  {ImGuiStyleVar_SelectableTextAlign,  true }},
};

// Maps JSON color name to ImGuiCol enum
static const std::unordered_map<std::string, ImGuiCol> k_styleColorTable = {
	{"Text",                  ImGuiCol_Text},
	{"TextDisabled",          ImGuiCol_TextDisabled},
	{"WindowBg",              ImGuiCol_WindowBg},
	{"ChildBg",               ImGuiCol_ChildBg},
	{"PopupBg",               ImGuiCol_PopupBg},
	{"Border",                ImGuiCol_Border},
	{"BorderShadow",          ImGuiCol_BorderShadow},
	{"FrameBg",               ImGuiCol_FrameBg},
	{"FrameBgHovered",        ImGuiCol_FrameBgHovered},
	{"FrameBgActive",         ImGuiCol_FrameBgActive},
	{"TitleBg",               ImGuiCol_TitleBg},
	{"TitleBgActive",         ImGuiCol_TitleBgActive},
	{"TitleBgCollapsed",      ImGuiCol_TitleBgCollapsed},
	{"MenuBarBg",             ImGuiCol_MenuBarBg},
	{"ScrollbarBg",           ImGuiCol_ScrollbarBg},
	{"ScrollbarGrab",         ImGuiCol_ScrollbarGrab},
	{"ScrollbarGrabHovered",  ImGuiCol_ScrollbarGrabHovered},
	{"ScrollbarGrabActive",   ImGuiCol_ScrollbarGrabActive},
	{"CheckMark",             ImGuiCol_CheckMark},
	{"SliderGrab",            ImGuiCol_SliderGrab},
	{"SliderGrabActive",      ImGuiCol_SliderGrabActive},
	{"Button",                ImGuiCol_Button},
	{"ButtonHovered",         ImGuiCol_ButtonHovered},
	{"ButtonActive",          ImGuiCol_ButtonActive},
	{"Header",                ImGuiCol_Header},
	{"HeaderHovered",         ImGuiCol_HeaderHovered},
	{"HeaderActive",          ImGuiCol_HeaderActive},
	{"Separator",             ImGuiCol_Separator},
	{"SeparatorHovered",      ImGuiCol_SeparatorHovered},
	{"SeparatorActive",       ImGuiCol_SeparatorActive},
	{"ResizeGrip",            ImGuiCol_ResizeGrip},
	{"ResizeGripHovered",     ImGuiCol_ResizeGripHovered},
	{"ResizeGripActive",      ImGuiCol_ResizeGripActive},
	{"Tab",                   ImGuiCol_Tab},
	{"TabHovered",            ImGuiCol_TabHovered},
	{"TabActive",             ImGuiCol_TabActive},
	{"TabUnfocused",          ImGuiCol_TabUnfocused},
	{"TabUnfocusedActive",    ImGuiCol_TabUnfocusedActive},
	{"PlotLines",             ImGuiCol_PlotLines},
	{"PlotLinesHovered",      ImGuiCol_PlotLinesHovered},
	{"PlotHistogram",         ImGuiCol_PlotHistogram},
	{"PlotHistogramHovered",  ImGuiCol_PlotHistogramHovered},
	{"TableHeaderBg",         ImGuiCol_TableHeaderBg},
	{"TableBorderStrong",     ImGuiCol_TableBorderStrong},
	{"TableBorderLight",      ImGuiCol_TableBorderLight},
	{"TableRowBg",            ImGuiCol_TableRowBg},
	{"TableRowBgAlt",         ImGuiCol_TableRowBgAlt},
	{"TextSelectedBg",        ImGuiCol_TextSelectedBg},
	{"DragDropTarget",        ImGuiCol_DragDropTarget},
	{"NavHighlight",          ImGuiCol_NavHighlight},
	{"NavWindowingHighlight", ImGuiCol_NavWindowingHighlight},
	{"NavWindowingDimBg",     ImGuiCol_NavWindowingDimBg},
	{"ModalWindowDimBg",      ImGuiCol_ModalWindowDimBg},
};

bool MkGuiStyleManager::startup(MkGuiContext* guiContext, const std::filesystem::path& stylesDir)
{
	m_guiContext = guiContext;

	if (!std::filesystem::exists(stylesDir))
	{
		MIKAN_LOG_WARNING("MkGuiStyleManager::startup") << "Styles directory does not exist: " << stylesDir;
		return true;  // Not a fatal error — styles are optional
	}

	bool bSuccess = true;
	for (const auto& entry : std::filesystem::directory_iterator(stylesDir))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".json")
		{
			if (!loadStyleFile(entry.path()))
			{
				bSuccess = false;
			}
		}
	}

	return bSuccess;
}

void MkGuiStyleManager::shutdown()
{
	m_styles.clear();
	m_guiContext = nullptr;
}

MkGuiStyleConstPtr MkGuiStyleManager::getStyle(const std::string& name) const
{
	auto it = m_styles.find(name);
	if (it == m_styles.end())
	{
		MIKAN_LOG_ERROR("MkGuiStyleManager::getStyle") << "Style not found: " << name;
		return nullptr;
	}
	return it->second;
}

bool MkGuiStyleManager::loadStyleFile(const std::filesystem::path& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		MIKAN_LOG_ERROR("MkGuiStyleManager::loadStyleFile") << "Failed to open style file: " << filePath;
		return false;
	}

	nlohmann::json root;
	try
	{
		root = nlohmann::json::parse(file);
	}
	catch (const nlohmann::json::parse_error& e)
	{
		MIKAN_LOG_ERROR("MkGuiStyleManager::loadStyleFile") << "JSON parse error in " << filePath << ": " << e.what();
		return false;
	}

	bool bSuccess = true;
	for (auto& [styleName, styleJson] : root.items())
	{
		auto style = std::make_shared<MkGuiStyle>();
		style->name = styleName;

		// Optional font
		if (styleJson.contains("font") && styleJson["font"].is_string())
		{
			style->fontName = styleJson["font"].get<std::string>();
		}

		// Style vars
		if (styleJson.contains("vars") && styleJson["vars"].is_array())
		{
			for (const auto& varJson : styleJson["vars"])
			{
				if (!varJson.contains("name") || !varJson.contains("value"))
					continue;

				const std::string varName = varJson["name"].get<std::string>();
				auto it = k_styleVarTable.find(varName);
				if (it == k_styleVarTable.end())
				{
					MIKAN_LOG_WARNING("MkGuiStyleManager::loadStyleFile") << "Unknown style var: " << varName;
					continue;
				}

				MkGuiStyleVarEntry entry;
				entry.var = it->second.first;
				entry.isVec2 = it->second.second;

				if (entry.isVec2 && varJson["value"].is_array() && varJson["value"].size() == 2)
				{
					entry.vec2Val = {varJson["value"][0].get<float>(), varJson["value"][1].get<float>()};
				}
				else if (!entry.isVec2 && varJson["value"].is_number())
				{
					entry.floatVal = varJson["value"].get<float>();
				}
				else
				{
					MIKAN_LOG_WARNING("MkGuiStyleManager::loadStyleFile") << "Mismatched value type for var: " << varName;
					continue;
				}

				style->vars.push_back(entry);
			}
		}

		// Style colors
		if (styleJson.contains("colors") && styleJson["colors"].is_array())
		{
			for (const auto& colorJson : styleJson["colors"])
			{
				if (!colorJson.contains("name") || !colorJson.contains("value"))
					continue;

				const std::string colorName = colorJson["name"].get<std::string>();
				auto it = k_styleColorTable.find(colorName);
				if (it == k_styleColorTable.end())
				{
					MIKAN_LOG_WARNING("MkGuiStyleManager::loadStyleFile") << "Unknown style color: " << colorName;
					continue;
				}

				if (!colorJson["value"].is_array() || colorJson["value"].size() != 4)
				{
					MIKAN_LOG_WARNING("MkGuiStyleManager::loadStyleFile") << "Color value must be [r,g,b,a] array for: " << colorName;
					continue;
				}

				MkGuiStyleColorEntry entry;
				entry.col = it->second;
				entry.value = {
					colorJson["value"][0].get<float>(),
					colorJson["value"][1].get<float>(),
					colorJson["value"][2].get<float>(),
					colorJson["value"][3].get<float>()
				};
				style->colors.push_back(entry);
			}
		}

		m_styles[styleName] = style;
	}

	return bSuccess;
}
