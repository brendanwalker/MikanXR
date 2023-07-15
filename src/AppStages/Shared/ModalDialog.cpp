#include "AppStage.h"
#include "ModalDialog.h"

ModalDialog::ModalDialog(class AppStage* ownerAppStage)
	: m_ownerAppStage(ownerAppStage)
{
}

ModalDialog::~ModalDialog()
{
}

Rml::Context* ModalDialog::getRmlContext() const
{
	return m_ownerAppStage->getRmlContext();
}
