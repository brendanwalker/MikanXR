//-- includes -----
#include "AppStage.h"
#include "ModalDialog_Confirm.h"

#include "imgui.h"

#include <assert.h>

//-- public methods -----
ModalDialog_Confirm::ModalDialog_Confirm(AppStage* appStage)
	: ModalDialog(appStage)
{
}

bool ModalDialog_Confirm::confirmQuestion(
	AppStage* appStage,
	const std::string& title,
	const std::string& question,
	ConfirmCallback acceptCallback,
	ConfirmCallback rejectCallback)
{
	ModalDialog_Confirm* confirmModal = appStage->pushModalDialog<ModalDialog_Confirm>();

	if (!confirmModal->init(title, question, acceptCallback, rejectCallback))
	{
		appStage->popModalDialog();
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
	m_title = title;
	m_question = question;
	m_acceptCallback = acceptCallback;
	m_rejectCallback = rejectCallback;
	return true;
}

void ModalDialog_Confirm::onGui()
{
	static const char* k_popupId = "##ConfirmModal";
	if (m_bNeedsOpen)
	{
		ImGui::OpenPopup(k_popupId);
		m_bNeedsOpen = false;
	}

	ImGui::SetNextWindowPos(
		ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(k_popupId, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(m_title.c_str());
		ImGui::Separator();
		ImGui::TextUnformatted(m_question.c_str());
		ImGui::Spacing();

		bool accepted = false, rejected = false;
		if (ImGui::Button("Yes")) { ImGui::CloseCurrentPopup(); accepted = true; }
		ImGui::SameLine();
		if (ImGui::Button("No"))  { ImGui::CloseCurrentPopup(); rejected = true; }
		ImGui::EndPopup();

		if (accepted) onAcceptQuestion();
		else if (rejected) onRejectQuestion();
	}
}

void ModalDialog_Confirm::onAcceptQuestion()
{
	ConfirmCallback callback = std::move(m_acceptCallback);
	AppStage* ownerAppStage = m_ownerAppStage;

	assert(ownerAppStage->getCurrentModalDialog() == this);
	ownerAppStage->popModalDialog(); // deletes 'this'

	if (callback)
		callback();
}

void ModalDialog_Confirm::onRejectQuestion()
{
	ConfirmCallback callback = std::move(m_rejectCallback);
	AppStage* ownerAppStage = m_ownerAppStage;

	assert(ownerAppStage->getCurrentModalDialog() == this);
	ownerAppStage->popModalDialog(); // deletes 'this'

	if (callback)
		callback();
}
