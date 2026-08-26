//-- includes -----
#include "AppStage.h"
#include "LocText.h"
#include "ModalDialog_MessageBox.h"

#include "imgui.h"

#include <assert.h>

//-- public methods -----
ModalDialog_MessageBox::ModalDialog_MessageBox(AppStage* appStage)
	: ModalDialog(appStage)
{
}

bool ModalDialog_MessageBox::showMessageBox(AppStage* appStage, const std::string& message,
											const std::string& buttonLabel, DismissCallback dismissCallback)
{
	ModalDialog_MessageBox* messageBoxModal= appStage->pushModalDialog<ModalDialog_MessageBox>();

	if (!messageBoxModal->init(message, buttonLabel, dismissCallback))
	{
		appStage->popModalDialog();
		return false;
	}

	return true;
}

bool ModalDialog_MessageBox::init(const std::string& message, const std::string& buttonLabel,
								  DismissCallback dismissCallback)
{
	m_message= message;
	m_buttonLabel= buttonLabel.empty() ? locLabel("common.ok") : buttonLabel;
	m_dismissCallback= dismissCallback;
	return true;
}

void ModalDialog_MessageBox::onGui()
{
	static const char* k_popupId= "##MessageBoxModal";
	if (m_bNeedsOpen)
	{
		ImGui::OpenPopup(k_popupId);
		m_bNeedsOpen= false;
	}

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(k_popupId, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(m_message.c_str());
		ImGui::Spacing();

		bool dismissed= false;
		if (ImGui::Button(m_buttonLabel.c_str()))
		{
			ImGui::CloseCurrentPopup();
			dismissed= true;
		}
		ImGui::EndPopup();

		if (dismissed)
			onDismiss();
	}
}

void ModalDialog_MessageBox::onDismiss()
{
	DismissCallback callback= std::move(m_dismissCallback);
	AppStage* ownerAppStage= m_ownerAppStage;

	assert(ownerAppStage->getCurrentModalDialog() == this);
	ownerAppStage->popModalDialog(); // deletes 'this'

	if (callback)
		callback();
}
