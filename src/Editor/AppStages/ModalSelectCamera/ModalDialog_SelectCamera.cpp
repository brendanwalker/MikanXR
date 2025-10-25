//-- inludes -----
#include "CameraObjectSystem.h"
#include "MainWindow.h"
#include "ModalDialog_SelectCamera.h"
#include "RmlModel_SelectCamera.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

#include <assert.h>

//-- public methods -----
ModalDialog_SelectCamera::ModalDialog_SelectCamera(AppStage* appStage)
	: ModalDialog(appStage)
	, m_selectCameraModel(new RmlModel_SelectCamera)
{
}

ModalDialog_SelectCamera::~ModalDialog_SelectCamera()
{
	m_ownerAppStage->removeRmlDocument(m_selectCameraView);
	m_selectCameraView = nullptr;

	m_selectCameraModel->dispose();
	delete m_selectCameraModel;
}

bool ModalDialog_SelectCamera::selectCamera(
	SelectCallback selectCallback,
	CancelCallback rejectCallback)
{
	// Allocate a new file browser modal dialog
	AppStage* ownerAppStage = MainWindow::getInstance()->getCurrentAppStage();
	ModalDialog_SelectCamera* confirmModal = ownerAppStage->pushModalDialog<ModalDialog_SelectCamera>();

	// Attempt to initialize the confirm modal
	if (!confirmModal->init(selectCallback, rejectCallback))
	{
		// On failure, destroy the modal dialog we just created
		ownerAppStage->popModalDialog();

		return false;
	}

	return true;
}

bool ModalDialog_SelectCamera::init(
	SelectCallback selectCallback,
	CancelCallback cancelCallback)
{
	// Finish model initialization
	CameraObjectSystemPtr cameraSystem = m_ownerAppStage->getSystemOfType<CameraObjectSystem>();
	if (!m_selectCameraModel->init(getRmlContext(), cameraSystem))
	{
		return false;
	}

	// Bind event delegates to model events
	m_selectCameraModel->OnOk = MakeDelegate(this, &ModalDialog_SelectCamera::onSelectCamera);
	m_selectCameraModel->OnCancel = MakeDelegate(this, &ModalDialog_SelectCamera::onCancel);

	// Bind event delegates to callbacks passed in
	m_selectCallback = selectCallback;
	m_cancelCallback = cancelCallback;

	// Create the view now that the model is safely initialized
	m_selectCameraView = m_ownerAppStage->addRmlDocument("select_camera.rml", false);

	return true;
}

void ModalDialog_SelectCamera::onSelectCamera(MikanCameraID cameraId)
{
	if (m_selectCallback)
	{
		m_selectCallback(cameraId);
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}

void ModalDialog_SelectCamera::onCancel()
{
	if (m_cancelCallback)
	{
		m_cancelCallback();
	}

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}