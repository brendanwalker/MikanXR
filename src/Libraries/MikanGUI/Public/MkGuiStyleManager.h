#pragma once

#include "MkGuiStyle.h"

#include <filesystem>
#include <string>
#include <unordered_map>

class MkGuiStyleManager
{
public:
	bool startup(class MkGuiContext* guiContext, const std::filesystem::path& stylesDir);
	void shutdown();

	MkGuiStyleConstPtr getStyle(const std::string& name) const;

private:
	bool loadStyleFile(const std::filesystem::path& filePath);

	class MkGuiContext* m_guiContext = nullptr;
	std::unordered_map<std::string, MkGuiStylePtr> m_styles;
};
