//-- inludes -----
#include "App.h"
#include "AppStage_FileBrowser.h"
#include "PathUtils.h"
#include "Renderer.h"
#include "Logger.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

//-- statics ----
const char* AppStage_FileBrowser::APP_STAGE_NAME = "FileBrowser";

struct FileBrowserEntry
{
	FileBrowserEntry(bool directory, int depth, Rml::String name)
		: visible(depth == 0), directory(directory), collapsed(true), depth(depth), name(std::move(name))
	{}

	bool visible;
	bool directory;
	bool collapsed;
	int depth;
	std::string name;
};

struct FileBrowserData
{
	std::string initialDirectory;
	std::string currentFilePath;
	std::vector<FileBrowserEntry> files;

	void BuildTree(const std::string& current_directory, int current_depth)
	{
		const Rml::StringList directories = PathUtils::listDirectories(current_directory);

		for (const Rml::String& directory : directories)
		{
			files.push_back(FileBrowserEntry(true, current_depth, directory));

			// Recurse into the child directory
			const std::string next_directory = current_directory + directory + '/';
			BuildTree(next_directory, current_depth + 1);
		}

		const Rml::StringList filenames = PathUtils::listFiles(current_directory);
		for (const std::string& filename : filenames)
		{
			files.push_back(FileBrowserEntry(false, current_depth, filename));
		}
	}

	void ToggleExpand(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters)
	{
		if (parameters.empty())
			return;

		// The index of the file/directory being toggled is passed in as the first parameter.
		const size_t toggle_index = (size_t)parameters[0].Get<int>();
		if (toggle_index >= files.size())
			return;

		FileBrowserEntry& toggle_file = files[toggle_index];

		const bool collapsing = !toggle_file.collapsed;
		const int depth = toggle_file.depth;

		toggle_file.collapsed = collapsing;

		// Loop through all descendent entries.
		for (size_t i = toggle_index + 1; i < files.size() && files[i].depth > depth; i++)
		{
			// Hide all items if we are collapsing. If instead we are expanding, make all direct children visible.
			files[i].visible = !collapsing && (files[i].depth == depth + 1);
			files[i].collapsed = true;
		}

		handle.DirtyVariable("files");
	}
};

//-- public methods -----
AppStage_FileBrowser::AppStage_FileBrowser(App* app)
	: AppStage(app, AppStage_FileBrowser::APP_STAGE_NAME)
	, m_data(new FileBrowserData)
{}

AppStage_FileBrowser::~AppStage_FileBrowser()
{
	delete m_data;
}

void AppStage_FileBrowser::setInitialDirectory(const std::string& initial_directory)
{
	m_data->initialDirectory= initial_directory;
}

void AppStage_FileBrowser::enter()
{
	AppStage::enter();

	m_data->BuildTree(m_data->initialDirectory, 0);

	Rml::DataModelConstructor constructor = getRmlContext()->CreateDataModel("filebrowser");
	if (!constructor)
		return;

	if (auto file_handle = constructor.RegisterStruct<FileBrowserEntry>())
	{
		file_handle.RegisterMember("visible", &FileBrowserEntry::visible);
		file_handle.RegisterMember("directory", &FileBrowserEntry::directory);
		file_handle.RegisterMember("collapsed", &FileBrowserEntry::collapsed);
		file_handle.RegisterMember("depth", &FileBrowserEntry::depth);
		file_handle.RegisterMember("name", &FileBrowserEntry::name);
	}

	constructor.RegisterArray<decltype(m_data->files)>();
	constructor.Bind("files", &m_data->files);
	constructor.BindEventCallback("toggle_expand", &FileBrowserData::ToggleExpand, m_data);

	pushRmlDocument("rml\\file_browser.rml");
}

void AppStage_FileBrowser::onRmlClickEvent(const std::string& value)
{
	if (value == "accept_filepath")
	{
		if (OnAcceptFilePath)
		{
			OnAcceptFilePath(m_data->currentFilePath);
		}
		m_app->popAppState();
	}
	else if (value == "reject_filepath")
	{
		if (OnRejectFilePath)
		{
			OnRejectFilePath();
		}
		m_app->popAppState();
	}
}