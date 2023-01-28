#include "RmlModel_FileBrowser.h"
#include "MathMikan.h"
#include "PathUtils.h"
#include "ProfileConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_FileBrowser::s_bHasRegisteredTypes = false;

RmlModel_FileBrowserEntry::RmlModel_FileBrowserEntry(
	bool directory, Rml::String name)
	: directory(directory)
	, name(std::move(name))
{}

void RmlModel_FileBrowser::setTitle(const Rml::String& title)
{
	m_title= title;
}

void RmlModel_FileBrowser::setInitialDirectory(const Rml::String& initialDirectory)
{
	m_initialDirectory= std::filesystem::path(initialDirectory);
}

void RmlModel_FileBrowser::setTypeFilter(const Rml::Vector<Rml::String>& typeFilters)
{
	m_typeFilters= typeFilters;
}

bool RmlModel_FileBrowser::init(Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "filebrowser");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto file_handle = constructor.RegisterStruct<RmlModel_FileBrowserEntry>())
		{
			file_handle.RegisterMember("directory", &RmlModel_FileBrowserEntry::directory);
			file_handle.RegisterMember("name", &RmlModel_FileBrowserEntry::name);
		}

		constructor.RegisterArray<decltype(m_files)>();
		s_bHasRegisteredTypes = true;
	}

	constructor.Bind("title", &m_title);
	constructor.Bind("files", &m_files);	
	constructor.Bind("current_dirpath", &m_currentDirectoryPathString);
	constructor.Bind("current_filepath", &m_currentFilePathString);
	constructor.BindEventCallback("select_file_entry", &RmlModel_FileBrowser::selectFileEntry, this);
	constructor.BindEventCallback(
		"exit_directory",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			// If the parent path generates the same path as the current path, we must be at the root
			// Pass in an empty path in this case to get a list of drives
			const auto parentPath= m_currentDirectoryPath.parent_path();
			setDirectoryPath(parentPath != m_currentDirectoryPath ? parentPath : std::filesystem::path());
		});
	constructor.BindEventCallback(
		"accept_filepath",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnAcceptFilePath) OnAcceptFilePath(m_currentFilePathString);
		});
	constructor.BindEventCallback(
		"reject_filepath",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnRejectFilePath) OnRejectFilePath();
		});

	setCurrentFilePath("");
	setDirectoryPath(m_initialDirectory);

	return true;
}

void RmlModel_FileBrowser::dispose()
{
	OnAcceptFilePath.Clear();
	OnRejectFilePath.Clear();
	RmlModel::dispose();
}

void RmlModel_FileBrowser::setDirectoryPath(const std::filesystem::path& newDirecoryPath)
{
	m_currentDirectoryPath = newDirecoryPath;
	m_currentDirectoryPathString = newDirecoryPath.string();
	m_modelHandle.DirtyVariable("current_dirpath");

	const Rml::StringList directories = PathUtils::listDirectories(m_currentDirectoryPathString);

	m_files.clear();

	for (const Rml::String& directory : directories)
	{
		m_files.push_back(RmlModel_FileBrowserEntry(true, directory));
	}

	if (m_typeFilters.size() > 0)
	{
		for (const std::string& extension : m_typeFilters)
		{
			const Rml::StringList filenames = PathUtils::listFiles(m_currentDirectoryPathString, extension);
			for (const std::string& filename : filenames)
			{
				m_files.push_back(RmlModel_FileBrowserEntry(false, filename));
			}
		}
	}
	else
	{
		const Rml::StringList filenames = PathUtils::listFiles(m_currentDirectoryPathString);
		for (const std::string& filename : filenames)
		{
			m_files.push_back(RmlModel_FileBrowserEntry(false, filename));
		}
	}

	m_modelHandle.DirtyVariable("files");
}

void RmlModel_FileBrowser::selectFileEntry(
	Rml::DataModelHandle handle, 
	Rml::Event& /*ev*/, 
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	// The index of the file/directory being toggled is passed in as the first parameter.
	const size_t toggle_index = (size_t)parameters[0].Get<int>();
	if (toggle_index >= m_files.size())
		return;

	const RmlModel_FileBrowserEntry& selectedFile = m_files[toggle_index];
	if (selectedFile.directory)
	{
		setDirectoryPath(m_currentDirectoryPath / selectedFile.name);
	}
	else
	{
		setCurrentFilePath((m_currentDirectoryPath / selectedFile.name).string());
	}	
}

void RmlModel_FileBrowser::setCurrentFilePath(const Rml::String& filepath)
{
	m_currentFilePathString= filepath;
	m_modelHandle.DirtyVariable("current_filepath");
}