#include "AppStage.h"
#include "Shared/GuiPanel_VRTrackingVolumeComponent.h"
#include "MarkerObjectSystem.h"
#include "TrackingMountObjectSystem.h"
#include "MikanCoreTypes.h"

#include "imgui.h"

bool GuiPanel_VRTrackingVolumeComponent::init()
{
	m_markerObjectSystem = getOwnerAppStage()->getSystemOfType<MarkerObjectSystem>();
	m_trackingMountObjectSystem = getOwnerAppStage()->getSystemOfType<TrackingMountObjectSystem>();

	return initTypedPropertyInterface<VRTrackingVolumeComponent>();
}

void GuiPanel_VRTrackingVolumeComponent::onGui()
{
	GuiPanel_MikanComponent::onGui();

	VRTrackingVolumeComponentPtr volumeComp = getVRTrackingVolumeComponent();
	if (!volumeComp)
		return;

	ImGui::Separator();

	MarkerObjectSystemPtr markerSystem = getMarkerObjectSystem();
	auto volumeDef = volumeComp->getVRTrackingVolumeDefinition();

	// Charuco mount dropdown (tracking mount IDs from this tracking volume)
	{
		int currentMountId = volumeDef->getCharucoTrackingMountId();
		const std::string previewStr = (currentMountId == INVALID_MIKAN_ID) ? "None" : std::to_string(currentMountId);

		if (ImGui::BeginCombo("Charuco Mount", previewStr.c_str()))
		{
			const auto& mountIds = volumeDef->getTrackingMountIDs();
			for (int mountId : mountIds)
			{
				const bool bSelected = (mountId == currentMountId);
				if (ImGui::Selectable(std::to_string(mountId).c_str(), bSelected))
				{
					addDeferredGuiEvent([volumeDef, mountId]() {
						volumeDef->setCharucoTrackingMountId(mountId);
					});
				}
			}
			ImGui::EndCombo();
		}
	}

	// Origin marker dropdown
	{
		int currentMarkerId = volumeDef->getOriginMarkerId();
		const std::string previewStr = (currentMarkerId == INVALID_MIKAN_ID) ? "None" : std::to_string(currentMarkerId);

		if (ImGui::BeginCombo("Origin Marker", previewStr.c_str()))
		{
			if (markerSystem)
			{
				for (const auto& it : markerSystem->getComponentMap())
				{
					const int markerId = (int)it.first;
					const bool bSelected = (markerId == currentMarkerId);
					if (ImGui::Selectable(std::to_string(markerId).c_str(), bSelected))
					{
						addDeferredGuiEvent([volumeDef, markerId]() {
							volumeDef->setOriginMarkerId(markerId);
						});
					}
				}
			}
			ImGui::EndCombo();
		}
	}

	// Utility marker dropdown
	{
		int currentMarkerId = volumeDef->getUtilityMarkerId();
		const std::string previewStr = (currentMarkerId == INVALID_MIKAN_ID) ? "None" : std::to_string(currentMarkerId);

		if (ImGui::BeginCombo("Utility Marker", previewStr.c_str()))
		{
			if (markerSystem)
			{
				for (const auto& it : markerSystem->getComponentMap())
				{
					const int markerId = (int)it.first;
					const bool bSelected = (markerId == currentMarkerId);
					if (ImGui::Selectable(std::to_string(markerId).c_str(), bSelected))
					{
						addDeferredGuiEvent([volumeDef, markerId]() {
							volumeDef->setUtilityMarkerId(markerId);
						});
					}
				}
			}
			ImGui::EndCombo();
		}
	}
}

MarkerObjectSystemPtr GuiPanel_VRTrackingVolumeComponent::getMarkerObjectSystem() const
{
	return m_markerObjectSystem.lock();
}

VRTrackingVolumeComponentPtr GuiPanel_VRTrackingVolumeComponent::getVRTrackingVolumeComponent() const
{
	return std::dynamic_pointer_cast<VRTrackingVolumeComponent>(m_component.lock());
}
