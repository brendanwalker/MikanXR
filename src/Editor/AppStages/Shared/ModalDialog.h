#pragma once

#include "RmlFwd.h"

class AppStage;

class ModalDialog
{
public:
	Rml::Context* getRmlContext() const;
	virtual void update() {}

protected:
	friend class AppStage;
	
	// Only allow creation and destruction from AppStage::pushModalDialog()/popModalDialog
	ModalDialog(AppStage* ownerAppStage);
	virtual ~ModalDialog();

	class AppStage* m_ownerAppStage= nullptr;
};
