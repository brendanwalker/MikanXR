#include "GuiPanel_ProjectTracking.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MarkerTrackingVolumeSystem.h"
#include "MikanCoreTypes.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "ProjectManager.h"
#include "Shared/GuiPanel_MarkerTrackingVolumeComponent.h"
#include "Shared/GuiPanel_TrackingMountComponent.h"
#include "Shared/GuiPanel_VRTrackingVolumeComponent.h"
#include "TrackingMountComponent.h"
#include "TrackingMountObjectSystem.h"
#include "TrackingVolumeQueries.h"
#include "VRTrackingVolumeComponent.h"
#include "VRTrackingVolumeSystem.h"

#include "imgui.h"

bool GuiPanel_ProjectTracking::init(ProjectGuiPanelContext* context)
{
	m_context = context;
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	m_projectManager = ownerAppStage->getProjectManager();
	m_trackingMountSystem = ownerAppStage->getObjectSystemOfType<TrackingMountObjectSystem>();

	// Auto-select first tracking volume if available
	auto pm = m_projectManager.lock();
	TrackingVolumeIdList volumeIds = TrackingVolumeQueries::getTrackingVolumeIdList(pm);
	if (!volumeIds.empty())
	{
		setSelectedTrackingVolumeId(volumeIds[0]);
	}

	return true;
}

void GuiPanel_ProjectTracking::dispose()
{
	GuiPanel::dispose();
}

TrackingMountObjectSystemPtr GuiPanel_ProjectTracking::getTrackingMountSystem() const
{
	return m_trackingMountSystem.lock();
}

MarkerTrackingVolumeComponentPtr GuiPanel_ProjectTracking::getSelectedMarkerTrackingVolume() const
{
	auto pm = m_projectManager.lock();
	auto sys = pm->getSystemOfType<MarkerTrackingVolumeSystem>();
	return sys->getTypedComponentById((MikanTrackingVolumeID)m_selectedTrackingVolumeId);
}

VRTrackingVolumeComponentPtr GuiPanel_ProjectTracking::getSelectedVRTrackingVolume() const
{
	auto pm = m_projectManager.lock();
	auto sys = pm->getSystemOfType<VRTrackingVolumeSystem>();
	return sys->getTypedComponentById((MikanTrackingVolumeID)m_selectedTrackingVolumeId);
}

TrackingMountComponentPtr GuiPanel_ProjectTracking::getSelectedTrackingMount() const
{
	TrackingMountObjectSystemPtr sys = getTrackingMountSystem();
	if (sys && m_selectedTrackingMountId != INVALID_MIKAN_ID)
		return sys->getTypedComponentById((MikanTrackingMountID)m_selectedTrackingMountId);
	return nullptr;
}

void GuiPanel_ProjectTracking::setSelectedTrackingVolumeId(MikanTrackingVolumeID volumeId)
{
	m_selectedTrackingVolumeId = (int)volumeId;
	m_selectedTrackingMountId = INVALID_MIKAN_ID;

	if (VRTrackingVolumeComponentPtr vrVolume = getSelectedVRTrackingVolume())
	{
		m_isVRTrackingVolume = true;
		m_context->getVRTrackingVolumePanel()->setComponent(vrVolume);
		m_context->getMarkerTrackingVolumePanel()->setComponent(nullptr);
	}
	else if (MarkerTrackingVolumeComponentPtr markerVolume = getSelectedMarkerTrackingVolume())
	{
		m_isVRTrackingVolume = false;
		m_context->getVRTrackingVolumePanel()->setComponent(nullptr);
		m_context->getMarkerTrackingVolumePanel()->setComponent(markerVolume);
	}
	else
	{
		m_isVRTrackingVolume = false;
		m_context->getVRTrackingVolumePanel()->setComponent(nullptr);
		m_context->getMarkerTrackingVolumePanel()->setComponent(nullptr);
	}

	m_context->getTrackingMountPanel()->setComponent(nullptr);
}

void GuiPanel_ProjectTracking::setSelectedTrackingMountId(MikanTrackingMountID mountId)
{
	m_selectedTrackingMountId = (int)mountId;

	if (TrackingMountComponentPtr mount = getSelectedTrackingMount())
		m_context->getTrackingMountPanel()->setComponent(mount);
	else
		m_context->getTrackingMountPanel()->setComponent(nullptr);
}

void GuiPanel_ProjectTracking::onGui()
{
	auto pm = m_projectManager.lock();
	if (!pm)
		return;

	// Tracking Volumes list (combined VR + marker)
	TrackingVolumeIdList volumeIds = TrackingVolumeQueries::getTrackingVolumeIdList(pm);

	// Validate selection
	bool volumeSelValid = false;
	for (MikanTrackingVolumeID id : volumeIds)
	{
		if ((int)id == m_selectedTrackingVolumeId)
		{
			volumeSelValid = true;
			break;
		}
	}
	if (!volumeSelValid && m_selectedTrackingVolumeId != INVALID_MIKAN_ID)
		setSelectedTrackingVolumeId(INVALID_MIKAN_ID);

	if (ImGui::Button("Add SteamVR Volume"))
	{
		addDeferredGuiEvent([this]() {
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<VRTrackingVolumeSystem>();
			VRTrackingVolumeComponentPtr vol = sys->addNewVRTrackingVolume(eTrackingRuntime::SteamVR);
			MikanTrackingVolumeID id = vol->getVRTrackingVolumeDefinition()->getTrackingVolumeId();
			setSelectedTrackingVolumeId(id);
			});
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Marker Volume"))
	{
		addDeferredGuiEvent([this]() {
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<MarkerTrackingVolumeSystem>();
			MarkerTrackingVolumeComponentPtr vol = sys->addNewObjectByTypedDefinition();
			MikanTrackingVolumeID id = vol->getMarkerTrackingVolumeDefinition()->getTrackingVolumeId();
			setSelectedTrackingVolumeId(id);
			});
	}

	ImGui::Text("Tracking Volumes");
	if (ImGui::BeginListBox("##TrackingVolumes", ImVec2(-1, 100)))
	{
		for (MikanTrackingVolumeID id : volumeIds)
		{
			TrackingVolumeComponentPtr vol = TrackingVolumeQueries::getTrackingVolumeById(pm, id);
			std::string label = vol ? (vol->getName().empty()
				? "Volume " + std::to_string((int)id)
				: vol->getName()) : "Volume " + std::to_string((int)id);
			label += "##vol" + std::to_string((int)id);

			bool selected = (m_selectedTrackingVolumeId == (int)id);
			if (ImGui::Selectable(label.c_str(), selected))
			{
				int newId = (int)id;
				addDeferredGuiEvent([this, newId]() {
					setSelectedTrackingVolumeId((MikanTrackingVolumeID)newId);
				});
			}
		}
		ImGui::EndListBox();
	}
	
	if (m_selectedTrackingVolumeId != INVALID_MIKAN_ID)
	{
		if (ImGui::Button("Remove Volume"))
		{
			addDeferredGuiEvent([this]() {
				auto pm = m_projectManager.lock();
				TrackingVolumeQueries::removeTrackingVolume(pm, (MikanTrackingVolumeID)m_selectedTrackingVolumeId);
			});

			m_context->getMarkerTrackingVolumePanel()->onGui();
			m_context->getVRTrackingVolumePanel()->onGui();
		}
	}

	// Tracking Mounts sub-list (only for VR volumes)
	if (m_isVRTrackingVolume && m_selectedTrackingVolumeId != INVALID_MIKAN_ID)
	{
		ImGui::Separator();
		ImGui::Text("Tracking Mounts");

		TrackingMountObjectSystemPtr mountSystem = getTrackingMountSystem();
		VRTrackingVolumeComponentPtr vrVolume = getSelectedVRTrackingVolume();

		if (vrVolume && mountSystem)
		{
			const auto& mountIds = vrVolume->getVRTrackingVolumeDefinition()->getTrackingMountIDs();

			// Validate mount selection
			bool mountSelValid = false;
			for (MikanTrackingMountID id : mountIds)
			{
				if ((int)id == m_selectedTrackingMountId)
				{
					mountSelValid = true;
					break;
				}
			}
			if (!mountSelValid && m_selectedTrackingMountId != INVALID_MIKAN_ID)
				setSelectedTrackingMountId(INVALID_MIKAN_ID);

			if (ImGui::Button("Add Mount"))
			{
				addDeferredGuiEvent([this]() {
					VRTrackingVolumeComponentPtr vrVol = getSelectedVRTrackingVolume();
					TrackingMountObjectSystemPtr mountSys = getTrackingMountSystem();
					if (vrVol && mountSys)
					{
						TrackingMountComponentPtr mount = mountSys->addNewObjectByTypedDefinition();
						MikanTrackingMountID mountId =
							mount->getTrackingMountDefinition()->getTrackingMountId();
						vrVol->getVRTrackingVolumeDefinition()->addTrackingMountID(mountId);
						setSelectedTrackingMountId(mountId);
					}
					});
			}

			if (ImGui::BeginListBox("##TrackingMounts", ImVec2(-1, 80)))
			{
				for (MikanTrackingMountID id : mountIds)
				{
					TrackingMountComponentPtr mount = mountSystem->getTypedComponentById(id);
					std::string label = (mount && !mount->getName().empty())
						? mount->getName()
						: ("Mount " + std::to_string((int)id));
					label += "##mount" + std::to_string((int)id);

					bool selected = (m_selectedTrackingMountId == (int)id);
					if (ImGui::Selectable(label.c_str(), selected))
					{
						int newId = (int)id;
						addDeferredGuiEvent([this, newId]() {
							setSelectedTrackingMountId((MikanTrackingMountID)newId);
						});
					}
				}
				ImGui::EndListBox();
			}

			if (m_selectedTrackingMountId != INVALID_MIKAN_ID)
			{
				if (ImGui::Button("Remove Mount"))
				{
					addDeferredGuiEvent([this]() {
						VRTrackingVolumeComponentPtr vrVol = getSelectedVRTrackingVolume();
						TrackingMountObjectSystemPtr mountSys = getTrackingMountSystem();
						if (vrVol && mountSys)
						{
							const auto def = vrVol->getVRTrackingVolumeDefinition();
							def->removeTrackingMountID((MikanTrackingMountID)m_selectedTrackingMountId);
							mountSys->removeObjectByPrimaryComponentId((MikanTrackingMountID)m_selectedTrackingMountId);
						}
					});
				}

				m_context->getTrackingMountPanel()->onGui();
			}
		}
	}
}
