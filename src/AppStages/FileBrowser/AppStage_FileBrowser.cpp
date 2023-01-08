//-- inludes -----
#include "App.h"
#include "FileBrowser/AppStage_FileBrowser.h"
#include "FileBrowser/RmlModel_FileBrowser.h"
#include "PathUtils.h"
#include "Renderer.h"
#include "Logger.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

//-- statics ----
const char* AppStage_FileBrowser::APP_STAGE_NAME = "FileBrowser";

//-- public methods -----
AppStage_FileBrowser::AppStage_FileBrowser(App* app)
	: AppStage(app, AppStage_FileBrowser::APP_STAGE_NAME)
	, m_filebrowserModel(new RmlModel_FileBrowser)
{}

AppStage_FileBrowser::~AppStage_FileBrowser()
{
	delete m_filebrowserModel;
}

bool AppStage_FileBrowser::browseFile(
	const std::string& title,
	const std::string& initialDirectory,
	const std::vector<std::string>& typeFilters,
	AcceptFilePathCallback acceptCallback,
	RejectFilePathCallback rejectCallback)
{
	App* app= App::getInstance();

	if (app->getCurrentAppStage()->getAppStageName() == AppStage_FileBrowser::APP_STAGE_NAME)
		return false;

	AppStage_FileBrowser* fileBrowser= app->pushAppStage<AppStage_FileBrowser>();
	fileBrowser->m_filebrowserModel->setTitle(title);
	fileBrowser->m_filebrowserModel->setInitialDirectory(initialDirectory);
	fileBrowser->m_filebrowserModel->setTypeFilter(typeFilters);
	fileBrowser->m_acceptCallback= acceptCallback;
	fileBrowser->m_rejectCallback= rejectCallback;

	return true;
}

void AppStage_FileBrowser::enter()
{
	AppStage::enter();

	Rml::Context* context = getRmlContext();

	m_filebrowserModel->init(context);
	m_filebrowserModel->OnAcceptFilePath = MakeDelegate(this, &AppStage_FileBrowser::onAcceptFilePath);
	m_filebrowserModel->OnRejectFilePath = MakeDelegate(this, &AppStage_FileBrowser::onRejectFilePath);
	m_filebrowserView = addRmlDocument("rml\\file_browser.rml");
}

void AppStage_FileBrowser::exit()
{
	m_filebrowserModel->dispose();
	AppStage::exit();
}

void AppStage_FileBrowser::onAcceptFilePath(const Rml::String& filepath)
{
	if (m_acceptCallback)
	{
		m_acceptCallback(filepath);
	}

	m_app->popAppState();
}

void AppStage_FileBrowser::onRejectFilePath()
{
	if (m_rejectCallback)
	{
		m_rejectCallback();
	}

	m_app->popAppState();
}