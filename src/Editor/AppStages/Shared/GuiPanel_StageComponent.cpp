#include "AppStage.h"
#include "Shared/GuiPanel_StageComponent.h"
#include "TrackingVolumeQueries.h"
#include "ProjectManager.h"
#include "MikanCoreTypes.h"

#include "imgui.h"

bool GuiPanel_StageComponent::init(AppStage* ownerAppStage)
{
	m_ownerAppStage = ownerAppStage;
	return initTypedPropertyInterface<StageComponent>(ownerAppStage);
}

void GuiPanel_StageComponent::render(float deltaSeconds)
{
	GuiPanel_MikanComponent::render(deltaSeconds);

	StageComponentPtr stageComp = getStageComponent();
	if (!stageComp)
		return;

	ImGui::Separator();

	// Tracking volume dropdown
	{
		ProjectManagerPtr projectManager = stageComp->getOwnerProjectManager();
		std::vector<int> trackingVolumeIds = TrackingVolumeQueries::getTrackingVolumeIdList(projectManager);

		int currentVolumeId = stageComp->getStageComponentDefinition()->getTrackingVolumeId();
		const std::string previewStr = (currentVolumeId == INVALID_MIKAN_ID) ? "None" : std::to_string(currentVolumeId);

		if (ImGui::BeginCombo("Tracking Volume", previewStr.c_str()))
		{
			for (int volumeId : trackingVolumeIds)
			{
				const bool bSelected = (volumeId == currentVolumeId);
				const std::string label = std::to_string(volumeId);

				if (ImGui::Selectable(label.c_str(), bSelected))
				{
					addUpdateCallback([stageComp, volumeId]() {
						stageComp->setTrackingVolumeId(volumeId);
					});
				}
			}
			ImGui::EndCombo();
		}
	}
}

StageComponentPtr GuiPanel_StageComponent::getStageComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::dynamic_pointer_cast<StageComponent>(component);
	}
	return nullptr;
}
