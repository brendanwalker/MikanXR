//-- includes -----
#include "AppStage.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "LocText.h"
#include "ModalDialog_SceneAddCompositor.h"
#include "SceneComponent.h"
#include "StageComponent.h"

#include "imgui.h"

#include <assert.h>
#include <algorithm>

//-- public methods -----
ModalDialog_SceneAddCompositor::ModalDialog_SceneAddCompositor(AppStage* appStage)
	: ModalDialog(appStage)
{
}

bool ModalDialog_SceneAddCompositor::selectNewCompositor(AppStage* appStage, SceneComponentPtr ownerScene,
														 SelectCallback selectCallback, CancelCallback cancelCallback)
{
	ModalDialog_SceneAddCompositor* dialog= appStage->pushModalDialog<ModalDialog_SceneAddCompositor>();

	if (!dialog->init(ownerScene, selectCallback, cancelCallback))
	{
		appStage->popModalDialog();
		return false;
	}

	return true;
}

bool ModalDialog_SceneAddCompositor::init(SceneComponentPtr ownerScene, SelectCallback selectCallback,
										  CancelCallback cancelCallback)
{
	m_selectCallback= selectCallback;
	m_cancelCallback= cancelCallback;

	CompositorObjectSystemPtr compositorSystem= m_ownerAppStage->getSystemOfType<CompositorObjectSystem>();
	if (!compositorSystem)
		return false;

	// Populate list: compositors belonging to the parent stage that aren't already in the scene
	StageComponentPtr stageComponent= ownerScene->getParentStage();
	if (stageComponent)
	{
		const MikanStageID stageId= stageComponent->getStageId();
		const std::vector<MikanCompositorID>& existingIDs= ownerScene->getOutputCompositorIDs();
		const std::vector<MikanCompositorID> availableIDs= compositorSystem->getCompositorIdListForStage(stageId);

		for (const MikanCompositorID& id : availableIDs)
		{
			auto it= std::find(existingIDs.begin(), existingIDs.end(), id);
			if (it == existingIDs.end())
			{
				m_compositorIds.push_back(id);
				CompositorComponentPtr comp= compositorSystem->getCompositorById(id);
				const std::string name= comp ? comp->getName() : "";
				m_compositorNames.push_back(name.empty() ? locFormat("project.compositorFallbackNameFmt", id) : name);
			}
		}
	}

	return true;
}

void ModalDialog_SceneAddCompositor::onGui()
{
	const std::string k_popupId= std::string(locText("windows.addCompositor")) + "##SceneAddCompositorModal";
	if (m_bNeedsOpen)
	{
		ImGui::OpenPopup(k_popupId.c_str());
		m_bNeedsOpen= false;
	}

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(k_popupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(locText("project.addCompositorToScene"));
		ImGui::Separator();

		auto itemGetter= [](void* data, int idx) -> const char*
		{
			const auto* names= static_cast<std::vector<std::string>*>(data);
			if (idx < 0 || idx >= (int)names->size())
				return nullptr;
			return (*names)[idx].c_str();
		};
		ImGui::ListBox("##compositors", &m_selectedIndex, itemGetter, &m_compositorNames, (int)m_compositorNames.size(),
					   8);

		ImGui::Spacing();

		bool selected= false, cancelled= false;
		const bool hasSelection= !m_compositorIds.empty();
		if (!hasSelection)
			ImGui::BeginDisabled();
		if (ImGui::Button(locLabel("common.ok")))
		{
			ImGui::CloseCurrentPopup();
			selected= true;
		}
		if (!hasSelection)
			ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			ImGui::CloseCurrentPopup();
			cancelled= true;
		}
		ImGui::EndPopup();

		if (selected)
			onSelectCompositor();
		else if (cancelled)
			onCancel();
	}
}

void ModalDialog_SceneAddCompositor::onSelectCompositor()
{
	if (m_selectCallback && !m_compositorIds.empty())
		m_selectCallback(m_compositorIds[m_selectedIndex]);

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}

void ModalDialog_SceneAddCompositor::onCancel()
{
	if (m_cancelCallback)
		m_cancelCallback();

	assert(m_ownerAppStage->getCurrentModalDialog() == this);
	m_ownerAppStage->popModalDialog();
}
