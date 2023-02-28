//-- inludes -----
#include "App.h"
#include "ModalDialog_FileBrowser.h"
#include "RmlModel_FileBrowser.h"
#include "PathUtils.h"
#include "Renderer.h"
#include "Logger.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

#include <assert.h>

//-- public methods -----
ModalDialog_FileBrowser::ModalDialog_FileBrowser(AppStage* appStage)
	: ModalDialog(appStage)
	, m_fileBrowserModel(new RmlModel_FileBrowser)
{
}

ModalDialog_FileBrowser::~ModalDialog_FileBrowser()
{
	m_ownerAppStage->removeRmlDocument(m_fileBrowserView);
	m_fileBrowserView= nullptr;

	m_fileBrowserModel->dispose();
	delete m_fileBrowserModel;
}

bool ModalDialog_FileBrowser::browseFile(
	const std::string& title,
	const std::filesystem::path& initialDirectory,
	const std::vector<std::string>& typeFilters,
	AcceptFilePathCallback acceptCallback,
	RejectFilePathCallback rejectCallback)
{
	// Allocate a new file browser modal dialog
	AppStage* ownerAppStage = App::getInstance()->getCurrentAppStage();
	ModalDialog_FileBrowser* fileBrowser = ownerAppStage->pushModalDialog<ModalDialog_FileBrowser>();

	// Attempt to initialize the filebrowser
	if (!fileBrowser->init(title, initialDirectory, typeFilters, acceptCallback, rejectCallback))
	{
		// On failure, destroy the modal dialog we just created
		ownerAppStage->popModalDialog();

		return false;
	}

	return true;
}

bool ModalDialog_FileBrowser::init(
	const std::string& title,
	const std::filesystem::path& initialDirectory,
	const std::vector<std::string>& typeFilters,
	AcceptFilePathCallback acceptCallback,
	RejectFilePathCallback rejectCallback)
{
	// Set UI properties on the model
	m_fileBrowserModel->setTitle(title);
	m_fileBrowserModel->setInitialDirectory(initialDirectory.string());
	m_fileBrowserModel->setTypeFilter(typeFilters);

	// Finish model initialization
	if (!m_fileBrowserModel->init(getRmlContext()))
		return false;

	// Bind event delegates to model events
	m_fileBrowserModel->OnAcceptFilePath = MakeDelegate(this, &ModalDialog_FileBrowser::onAcceptFilePath);
	m_fileBrowserModel->OnRejectFilePath = MakeDelegate(this, &ModalDialog_FileBrowser::onRejectFilePath);

	// Bind event delegates to callbacks passed in
	m_acceptCallback = acceptCallback;
	m_rejectCallback = rejectCallback;

	// Create the view now that the model is safely initialized
	m_fileBrowserView = m_ownerAppStage->addRmlDocument("file_browser.rml", false);

	return true;
}

void ModalDialog_FileBrowser::onAcceptFilePath(const std::string& filepath)
{
	if (m_acceptCallback)
	{
		m_acceptCallback(filepath);
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}

void ModalDialog_FileBrowser::onRejectFilePath()
{
	if (m_rejectCallback)
	{
		m_rejectCallback();
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}