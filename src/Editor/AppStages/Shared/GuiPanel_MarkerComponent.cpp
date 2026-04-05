#include "AppStage.h"
#include "MarkerComponent.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "MarkerObjectSystem.h"

#include "imgui.h"

bool GuiPanel_MarkerComponent::init(AppStage* ownerAppStage)
{
	m_markerObjectSystem = ownerAppStage->getProjectManager()->getSystemOfType<MarkerObjectSystem>();
	return initTypedPropertyInterface<MarkerComponent>(ownerAppStage);
}

bool GuiPanel_MarkerComponent::setComponent(MikanComponentPtr component)
{
	if (GuiPanel_MikanComponent::setComponent(component))
	{
		MarkerComponentPtr markerComp = getMarkerComponent();
		if (markerComp && OnMarkerSelected)
		{
			OnMarkerSelected(markerComp->getMarkerDefinition()->getArucoId());
		}
		return true;
	}
	return false;
}

void GuiPanel_MarkerComponent::onGui()
{
	GuiPanel_MikanComponent::onGui();

	MarkerComponentPtr markerComp = getMarkerComponent();
	if (!markerComp)
		return;

	ImGui::Separator();

	// Aruco ID dropdown
	{
		int currentArucoId = markerComp->getMarkerDefinition()->getArucoId();

		std::vector<int> arucoIds;
		auto markerSystem = getMarkerObjectSystem();
		if (markerSystem)
		{
			for (const auto& it : markerSystem->getComponentMap())
			{
				auto markerCompEntry = it.second.lock();
				if (markerCompEntry)
				{
					auto markerCompTyped = std::static_pointer_cast<MarkerComponent>(markerCompEntry);
					arucoIds.push_back(markerCompTyped->getMarkerDefinition()->getArucoId());
				}
			}
		}

		const std::string previewStr = std::to_string(currentArucoId);
		if (ImGui::BeginCombo("Aruco ID", previewStr.c_str()))
		{
			for (int arucoId : arucoIds)
			{
				const bool bSelected = (arucoId == currentArucoId);
				if (ImGui::Selectable(std::to_string(arucoId).c_str(), bSelected))
				{
					addDeferredGuiEvent([this, markerComp, arucoId]() {
						markerComp->getMarkerDefinition()->setArucoId(arucoId);
						if (OnMarkerSelected)
						{
							OnMarkerSelected(arucoId);
						}
					});
				}
			}
			ImGui::EndCombo();
		}
	}
}

MarkerObjectSystemPtr GuiPanel_MarkerComponent::getMarkerObjectSystem() const
{
	return m_markerObjectSystem.lock();
}

MarkerObjectSystemDefinitionPtr GuiPanel_MarkerComponent::getMarkerObjectSystemDefinition() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getTypedDefinition();
	}
	return nullptr;
}

MarkerComponentPtr GuiPanel_MarkerComponent::getMarkerComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<MarkerComponent>(component);
	}
	return nullptr;
}
