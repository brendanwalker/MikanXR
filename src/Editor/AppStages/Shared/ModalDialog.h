#pragma once

class ModalDialog
{
public:
	virtual void onGui() {}

protected:
	friend class AppStage;

	// Only allow creation and destruction from AppStage::pushModalDialog()/popModalDialog
	ModalDialog(AppStage* ownerAppStage);
	virtual ~ModalDialog();

	class AppStage* m_ownerAppStage= nullptr;
	bool m_bNeedsOpen = true;
};
