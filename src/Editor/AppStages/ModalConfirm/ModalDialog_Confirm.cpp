//-- inludes -----
#include "MainWindow.h"
#include "ModalDialog_Confirm.h"
#include "RmlModel_Confirm.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

#include <assert.h>

//-- public methods -----
ModalDialog_Confirm::ModalDialog_Confirm(AppStage* appStage)
	: ModalDialog(appStage)
	, m_confirmModel(new RmlModel_Confirm)
{
}

ModalDialog_Confirm::~ModalDialog_Confirm()
{
	m_ownerAppStage->removeRmlDocument(m_confirmView);
	m_confirmView= nullptr;

	m_confirmModel->dispose();
	delete m_confirmModel;
}

bool ModalDialog_Confirm::confirmQuestion(
	const std::string& title,
	const std::string& question,
	ConfirmCallback acceptCallback,
	ConfirmCallback rejectCallback)
{
	// Allocate a new file browser modal dialog
	AppStage* ownerAppStage = MainWindow::getInstance()->getCurrentAppStage();
	ModalDialog_Confirm* confirmModal = ownerAppStage->pushModalDialog<ModalDialog_Confirm>();

	// Attempt to initialize the confirm modal
	if (!confirmModal->init(title, question, acceptCallback, rejectCallback))
	{
		// On failure, destroy the modal dialog we just created
		ownerAppStage->popModalDialog();

		return false;
	}

	return true;
}

bool ModalDialog_Confirm::init(
	const std::string& title,
	const std::string& question,
	ConfirmCallback acceptCallback,
	ConfirmCallback rejectCallback)
{
	// Finish model initialization
	if (!m_confirmModel->init(getRmlContext()))
		return false;

	// Set UI properties on the model
	m_confirmModel->setTitle(title);
	m_confirmModel->setQuestion(question);

	// Bind event delegates to model events
	m_confirmModel->OnAcceptQuestion = MakeDelegate(this, &ModalDialog_Confirm::onAcceptQuestion);
	m_confirmModel->OnRejectQuestion = MakeDelegate(this, &ModalDialog_Confirm::onRejectQuestion);

	// Bind event delegates to callbacks passed in
	m_acceptCallback = acceptCallback;
	m_rejectCallback = rejectCallback;

	// Create the view now that the model is safely initialized
	m_confirmView = m_ownerAppStage->addRmlDocument("modal_confirm.rml", false);

	return true;
}

void ModalDialog_Confirm::onAcceptQuestion()
{
	if (m_acceptCallback)
	{
		m_acceptCallback();
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}

void ModalDialog_Confirm::onRejectQuestion()
{
	if (m_rejectCallback)
	{
		m_rejectCallback();
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}