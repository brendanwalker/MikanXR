#pragma once

//-- includes -----
#include "Shared/ModalDialog.h"

#include <string>
#include <functional>

class AppStage;

//-- definitions -----

class ModalDialog_MessageBox : public ModalDialog
{
public:
	ModalDialog_MessageBox(AppStage* ownerAppStage);
	virtual ~ModalDialog_MessageBox() = default;

	using DismissCallback = std::function<void()>;
	static bool showMessageBox(
		AppStage* appStage,
		const std::string& message,
		const std::string& buttonLabel = "OK",
		DismissCallback dismissCallback = {});

	virtual void onGui() override;

protected:
	DismissCallback m_dismissCallback;

	std::string m_message;
	std::string m_buttonLabel;

	bool init(
		const std::string& message,
		const std::string& buttonLabel,
		DismissCallback dismissCallback);
	void onDismiss();
};
