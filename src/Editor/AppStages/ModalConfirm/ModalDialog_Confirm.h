#pragma once

//-- includes -----
#include "Shared/ModalDialog.h"

#include <string>
#include <functional>

class AppStage;

//-- definitions -----

class ModalDialog_Confirm : public ModalDialog
{
public:
	ModalDialog_Confirm(AppStage* ownerAppStage);
	virtual ~ModalDialog_Confirm()= default;

	using ConfirmCallback= std::function<void()>;
	static bool confirmQuestion(AppStage* appStage, const std::string& title, const std::string& question,
								ConfirmCallback acceptCallback= {}, ConfirmCallback rejectCallback= {});

	virtual void onGui() override;

protected:
	ConfirmCallback m_acceptCallback;
	ConfirmCallback m_rejectCallback;

	std::string m_title;
	std::string m_question;

	bool init(const std::string& title, const std::string& question, ConfirmCallback acceptCallback,
			  ConfirmCallback rejectCallback);
	void onAcceptQuestion();
	void onRejectQuestion();
};
