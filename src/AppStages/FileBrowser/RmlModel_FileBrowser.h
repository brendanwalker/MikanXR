#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include <filesystem>

struct RmlModel_FileBrowserEntry
{
	RmlModel_FileBrowserEntry(bool directory, Rml::String name);

	bool directory;
	Rml::String name;
};

class RmlModel_FileBrowser : public RmlModel
{
public:
	void setTitle(const Rml::String& title);
	void setInitialDirectory(const Rml::String& initialDirectory);
	void setTypeFilter(const Rml::Vector<Rml::String> &typeFilters);

	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	SinglecastDelegate<void(const std::filesystem::path& filepath)> OnAcceptFilePath;
	SinglecastDelegate<void()> OnRejectFilePath;

protected:
	Rml::String m_title;
	std::filesystem::path m_initialDirectory;
	std::filesystem::path m_currentDirectoryPath;
	Rml::String m_currentDirectoryPathString;
	Rml::String m_currentFilePathString;
	Rml::Vector<RmlModel_FileBrowserEntry> m_files;
	Rml::Vector<Rml::String> m_typeFilters;

	void setDirectoryPath(const std::filesystem::path& newDirecoryPath);
	void selectFileEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void setCurrentFilePath(const Rml::String& filepath);

	static bool s_bHasRegisteredTypes;
};
