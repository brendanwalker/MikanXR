#include "AppStage.h"
#include "MarkerTrackingVolumeComponent.h"
#include "Shared/GuiPanel_MarkerTrackingVolumeComponent.h"
#include "MarkerObjectSystem.h"
#include "TrackingMountObjectSystem.h"
#include "MikanCoreTypes.h"

#include "imgui.h"

bool GuiPanel_MarkerTrackingVolumeComponent::init(AppStage* ownerAppStage)
{
	m_markerObjectSystem = ownerAppStage->getObjectSystemOfType<MarkerObjectSystem>();
	m_trackingMountObjectSystem = ownerAppStage->getObjectSystemOfType<TrackingMountObjectSystem>();

	return initTypedPropertyInterface<MarkerTrackingVolumeComponent>(ownerAppStage);
}

void GuiPanel_MarkerTrackingVolumeComponent::render(float deltaSeconds)
{
	GuiPanel_MikanComponent::render(deltaSeconds);

	MarkerTrackingVolumeComponentPtr volumeComp = getMarkerTrackingVolumeComponent();
	if (!volumeComp)
		return;

	ImGui::Separator();

	// Origin marker dropdown
	{
		MarkerObjectSystemPtr markerSystem = getMarkerObjectSystem();
		int currentMarkerId = volumeComp->getMarkerTrackingVolumeDefinition()->getOriginMarkerId();
		const std::string previewStr = (currentMarkerId == INVALID_MIKAN_ID) ? "None" : std::to_string(currentMarkerId);

		if (ImGui::BeginCombo("Origin Marker", previewStr.c_str()))
		{
			if (markerSystem)
			{
				auto markerSystemDef = markerSystem->getTypedDefinition();
				for (const auto& it : markerSystem->getComponentMap())
				{
					const int markerId = (int)it.first;
					const bool bSelected = (markerId == currentMarkerId);

					if (ImGui::Selectable(std::to_string(markerId).c_str(), bSelected))
					{
						addUpdateCallback([volumeComp, markerId]() {
							volumeComp->getMarkerTrackingVolumeDefinition()->setOriginMarkerId(markerId);
						});
					}
				}
			}
			ImGui::EndCombo();
		}
	}
}

MarkerObjectSystemPtr GuiPanel_MarkerTrackingVolumeComponent::getMarkerObjectSystem() const
{
	return m_markerObjectSystem.lock();
}

TrackingMountObjectSystemPtr GuiPanel_MarkerTrackingVolumeComponent::getTrackingMountObjectSystem() const
{
	return m_trackingMountObjectSystem.lock();
}

MarkerTrackingVolumeComponentPtr GuiPanel_MarkerTrackingVolumeComponent::getMarkerTrackingVolumeComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::dynamic_pointer_cast<MarkerTrackingVolumeComponent>(component);
	}
	return nullptr;
}
