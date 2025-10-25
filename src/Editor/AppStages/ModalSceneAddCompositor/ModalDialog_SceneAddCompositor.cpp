//-- inludes -----
#include "CompositorObjectSystem.h"
#include "SceneComponent.h"
#include "MainWindow.h"
#include "ModalDialog_SceneAddCompositor.h"
#include "RmlModel_SceneAddCompositor.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

#include <assert.h>

//-- public methods -----
ModalDialog_SceneAddCompositor::ModalDialog_SceneAddCompositor(
	AppStage* appStage)
	: ModalDialog(appStage)
	, m_selectCompositorModel(new RmlModel_SceneAddCompositor)
{
}

ModalDialog_SceneAddCompositor::~ModalDialog_SceneAddCompositor()
{
	m_ownerAppStage->removeRmlDocument(m_selectCompositorView);
	m_selectCompositorView = nullptr;

	m_selectCompositorModel->dispose();
	delete m_selectCompositorModel;
}

bool ModalDialog_SceneAddCompositor::selectNewCompositor(
	SceneComponentPtr ownerScene,
	SelectCallback selectCallback,
	CancelCallback rejectCallback)
{
	// Allocate a new modal dialog
	AppStage* ownerAppStage = MainWindow::getInstance()->getCurrentAppStage();
	ModalDialog_SceneAddCompositor* confirmModal = ownerAppStage->pushModalDialog<ModalDialog_SceneAddCompositor>();

	// Attempt to initialize the confirm modal
	if (!confirmModal->init(ownerScene, selectCallback, rejectCallback))
	{
		// On failure, destroy the modal dialog we just created
		ownerAppStage->popModalDialog();

		return false;
	}

	return true;
}

bool ModalDialog_SceneAddCompositor::init(
	SceneComponentPtr ownerScene,
	SelectCallback selectCallback,
	CancelCallback cancelCallback)
{
	// Finish model initialization
	CompositorObjectSystemPtr compositorSystem = m_ownerAppStage->getSystemOfType<CompositorObjectSystem>();
	if (!m_selectCompositorModel->init(getRmlContext(), compositorSystem, ownerScene))
	{
		return false;
	}

	// Bind event delegates to model events
	m_selectCompositorModel->OnOk = MakeDelegate(this, &ModalDialog_SceneAddCompositor::onSelectCompositor);
	m_selectCompositorModel->OnCancel = MakeDelegate(this, &ModalDialog_SceneAddCompositor::onCancel);

	// Bind event delegates to callbacks passed in
	m_selectCallback = selectCallback;
	m_cancelCallback = cancelCallback;

	// Create the view now that the model is safely initialized
	m_selectCompositorView = m_ownerAppStage->addRmlDocument("scene_add_compositor.rml", false);

	return true;
}

void ModalDialog_SceneAddCompositor::onSelectCompositor(MikanCompositorID compositorId)
{
	if (m_selectCallback)
	{
		m_selectCallback(compositorId);
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}

void ModalDialog_SceneAddCompositor::onCancel()
{
	if (m_cancelCallback)
	{
		m_cancelCallback();
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}