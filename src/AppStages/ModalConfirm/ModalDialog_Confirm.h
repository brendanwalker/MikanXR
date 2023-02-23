#pragma once

//-- includes -----
#include "Shared/ModalDialog.h"
#include "SinglecastDelegate.h"

#include <string>
#include <vector>
#include <functional>

class AppStage;

namespace Rml
{
	class ElementDocument;
}

//-- definitions -----

class ModalDialog_Confirm : public ModalDialog
{
public:
	ModalDialog_Confirm(AppStage* ownerAppStage);
	virtual ~ModalDialog_Confirm();

	using ConfirmCallback = std::function<void()>;
	static bool confirmQuestion(
		const std::string& title,
		const std::string& question,
		ConfirmCallback acceptCallback={},
		ConfirmCallback rejectCallback={});

protected:
	class RmlModel_Confirm* m_confirmModel = nullptr;
	Rml::ElementDocument* m_confirmView = nullptr;

	ConfirmCallback m_acceptCallback;
	ConfirmCallback m_rejectCallback;

	bool init(
		const std::string& title,
		const std::string& question,
		ConfirmCallback acceptCallback,
		ConfirmCallback rejectCallback);
	void onAcceptQuestion();
	void onRejectQuestion();
};
