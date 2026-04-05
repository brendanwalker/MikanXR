#include "AppStage.h"
#include "Shared/GuiPanel_CameraComponent.h"
#include "VRTrackingVolumeComponent.h"
#include "VideoSourceQueries.h"
#include "ProjectManager.h"
#include "MikanCoreTypes.h"

#include "imgui.h"

bool GuiPanel_CameraComponent::init()
{
	return initTypedPropertyInterface<CameraComponent>();
}

bool GuiPanel_CameraComponent::setComponent(MikanComponentPtr component)
{
	if (GuiPanel_MikanComponent::setComponent(component))
	{
		m_trackingMountIdList.clear();
		m_videoSourceIdList.clear();

		CameraComponentPtr cameraComp = getCameraComponent();
		if (cameraComp)
		{
			// Tracking mount IDs from the owning VR tracking volume
			auto vrVolumeDefinition = cameraComp->getVRTrackingVolumeDefinitionMutable();
			if (vrVolumeDefinition)
			{
				m_trackingMountIdList = vrVolumeDefinition->getTrackingMountIDs();
			}

			// Video source IDs from query
			ProjectManagerPtr projectManager = cameraComp->getOwnerProjectManager();
			m_videoSourceIdList = VideoSourceQueries::getVideoSourceIdList(projectManager);
		}

		return true;
	}

	return false;
}

void GuiPanel_CameraComponent::onGui()
{
	GuiPanel_MikanComponent::onGui();

	CameraComponentPtr cameraComp = getCameraComponent();
	if (!cameraComp)
		return;

	ImGui::Separator();

	// Video source dropdown
	{
		int currentVideoSourceId = cameraComp->getCameraDefinition()->getVideoSourceId();
		const std::string previewStr = (currentVideoSourceId == INVALID_MIKAN_ID) ? "None" : std::to_string(currentVideoSourceId);

		// Refresh list on open
		ProjectManagerPtr projectManager = cameraComp->getOwnerProjectManager();
		std::vector<int> videoSourceIds = VideoSourceQueries::getVideoSourceIdList(projectManager);

		if (ImGui::BeginCombo("Video Source", previewStr.c_str()))
		{
			for (int videoSourceId : videoSourceIds)
			{
				const bool bSelected = (videoSourceId == currentVideoSourceId);
				const std::string label = std::to_string(videoSourceId);

				if (ImGui::Selectable(label.c_str(), bSelected))
				{
					addDeferredGuiEvent([cameraComp, videoSourceId]() {
						cameraComp->getCameraDefinition()->setVideoSourceId(videoSourceId);
					});
				}
			}
			ImGui::EndCombo();
		}
	}

	// Tracking mount dropdown
	{
		int currentMountId = cameraComp->getCameraDefinition()->getTrackingMountId();
		const std::string previewStr = (currentMountId == INVALID_MIKAN_ID) ? "None" : std::to_string(currentMountId);

		if (ImGui::BeginCombo("Tracking Mount", previewStr.c_str()))
		{
			// Refresh tracking mount list from VR tracking volume
			auto vrVolumeDefinition = cameraComp->getVRTrackingVolumeDefinitionMutable();
			std::vector<int> mountIds = vrVolumeDefinition ? vrVolumeDefinition->getTrackingMountIDs() : std::vector<int>{};

			for (int mountId : mountIds)
			{
				const bool bSelected = (mountId == currentMountId);
				const std::string label = std::to_string(mountId);

				if (ImGui::Selectable(label.c_str(), bSelected))
				{
					addDeferredGuiEvent([cameraComp, mountId]() {
						cameraComp->getCameraDefinition()->setTrackingMountId(mountId);
					});
				}
			}
			ImGui::EndCombo();
		}
	}
}

CameraComponentPtr GuiPanel_CameraComponent::getCameraComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<CameraComponent>(component);
	}
	return nullptr;
}
