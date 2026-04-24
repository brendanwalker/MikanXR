#pragma once

#include "MkGuiExport.h"
#include "IMkGuiStyle.h"

#include <filesystem>
#include <string>

class MIKAN_GUI_CLASS MkGuiStyleManager
{
public:
	MkGuiStyleManager();
	~MkGuiStyleManager();

	bool startup(class MkGuiContext* guiContext, const std::filesystem::path& stylesDir);
	void shutdown();

	MkGuiStyleConstPtr getStyle(const std::string& name) const;

private:
	bool loadStyleFile(const std::filesystem::path& filePath);

	struct Impl;
	Impl* m_impl = nullptr;
};
