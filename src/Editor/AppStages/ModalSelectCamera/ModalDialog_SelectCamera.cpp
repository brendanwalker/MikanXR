//-- includes -----
#include "AppStage.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "ModalDialog_SelectCamera.h"

#include "imgui.h"

#include <assert.h>

//-- public methods -----
ModalDialog_SelectCamera::ModalDialog_SelectCamera(AppStage* appStage)
	: ModalDialog(appStage)
{
}

bool ModalDialog_SelectCamera::selectCamera(
	AppStage* appStage,
	SelectCallback selectCallback,
	CancelCallback cancelCallback)
{
	ModalDialog_SelectCamera* dialog = appStage->pushModalDialog<ModalDialog_SelectCamera>();

	if (!dialog->init(selectCallback, cancelCallback))
	{
		appStage->popModalDialog();
		return false;
	}

	return true;
}

bool ModalDialog_SelectCamera::init(
	SelectCallback selectCallback,
	CancelCallback cancelCallback)
{
	m_selectCallback = selectCallback;
	m_cancelCallback = cancelCallback;

	CameraObjectSystemPtr cameraSystem = m_ownerAppStage->getSystemOfType<CameraObjectSystem>();
	if (!cameraSystem)
		return false;

	for (const auto& [id, weakPtr] : cameraSystem->getComponentMap())
	{
		CameraComponentPtr camera = std::static_pointer_cast<CameraComponent>(weakPtr.lock());
		if (!camera)
			continue;

		m_cameraIds.push_back((MikanCameraID)id);
		const std::string name = camera->getName();
		m_cameraNames.push_back(name.empty() ? ("Camera " + std::to_string(id)) : name);
	}

	return true;
}

void ModalDialog_SelectCamera::onGui()
{
	static const char* k_popupId = "Select Camera##SelectCameraModal";
	if (m_bNeedsOpen)
	{
		ImGui::OpenPopup(k_popupId);
		m_bNeedsOpen = false;
	}

	ImGui::SetNextWindowPos(
		ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(k_popupId, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Select Calibration Camera");
		ImGui::Separator();

		auto itemGetter = [](void* data, int idx, const char** out) -> bool {
			const auto* names = static_cast<std::vector<std::string>*>(data);
			if (idx < 0 || idx >= (int)names->size()) return false;
			*out = (*names)[idx].c_str();
			return true;
		};
		ImGui::ListBox("##cameras", &m_selectedIndex, itemGetter, &m_cameraNames, (int)m_cameraNames.size(), 8);

		ImGui::Spacing();

		bool selected = false, cancelled = false;
		const bool hasSelection = !m_cameraIds.empty();
		if (!hasSelection) ImGui::BeginDisabled();
		if (ImGui::Button("Ok"))     { ImGui::CloseCurrentPopup(); selected = true; }
		if (!hasSelection) ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); cancelled = true; }
		ImGui::EndPopup();

		if (selected)  onSelectCamera();
		else if (cancelled) onCancel();
	}
}

void ModalDialog_SelectCamera::onSelectCamera()
{
	if (m_selectCallback && !m_cameraIds.empty())
		m_selectCallback(m_cameraIds[m_selectedIndex]);

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}

void ModalDialog_SelectCamera::onCancel()
{
	if (m_cancelCallback)
		m_cancelCallback();

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}
