//-- inludes -----
#include "App.h"
#include "ModalDialog_Snap.h"
#include "RmlModel_Snap.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

#include <assert.h>

//-- public methods -----
ModalDialog_Snap::ModalDialog_Snap(AppStage* appStage)
	: ModalDialog(appStage)
	, m_snapModel(new RmlModel_Snap)
{
}

ModalDialog_Snap::~ModalDialog_Snap()
{
	m_ownerAppStage->removeRmlDocument(m_snapView);
	m_snapView= nullptr;

	m_snapModel->dispose();
	delete m_snapModel;
}

bool ModalDialog_Snap::selectSnapTarget(
	MikanSpatialFastenerID sourceId,
	SnapCallback snapCallback,
	CancelCallback cancelCallback)
{
	// Allocate a new file browser modal dialog
	App* app= App::getInstance();
	AppStage* ownerAppStage = app->getCurrentAppStage();
	ModalDialog_Snap* confirmModal = ownerAppStage->pushModalDialog<ModalDialog_Snap>();

	// Attempt to initialize the confirm modal
	if (!confirmModal->init(sourceId, snapCallback, cancelCallback))
	{
		// On failure, destroy the modal dialog we just created
		ownerAppStage->popModalDialog();

		return false;
	}

	return true;
}

bool ModalDialog_Snap::init(
	MikanSpatialFastenerID sourceId,
	SnapCallback snapCallback,
	CancelCallback cancelCallback)
{
	// Finish model initialization
	if (!m_snapModel->init(getRmlContext(), sourceId))
		return false;

	// Bind event delegates to model events
	m_snapModel->OnRequestFastenerSnap = MakeDelegate(this, &ModalDialog_Snap::onRequestFastenerSnap);
	m_snapModel->OnCancelFastenerSnap = MakeDelegate(this, &ModalDialog_Snap::onCancelSnap);

	// Bind event delegates to callbacks passed in
	m_requestSnapCallback = snapCallback;
	m_cancelCallback = cancelCallback;

	// Create the view now that the model is safely initialized
	m_snapView = m_ownerAppStage->addRmlDocument("modal_snap.rml", false);

	return true;
}

void ModalDialog_Snap::onRequestFastenerSnap(int sourceFastenerId, int targetFastenerId)
{
	if (m_requestSnapCallback)
	{
		m_requestSnapCallback(sourceFastenerId, targetFastenerId);
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}

void ModalDialog_Snap::onCancelSnap()
{
	if (m_cancelCallback)
	{
		m_cancelCallback();
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}