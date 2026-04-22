#include "GuiPanel_ProjectTracking.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MarkerTrackingVolumeSystem.h"
#include "MikanCoreTypes.h"
#include "IconsForkAwesome.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "ProjectManager.h"
#include "Shared/GuiPanel_MarkerTrackingVolumeComponent.h"
#include "Shared/GuiPanel_TrackingMountComponent.h"
#include "Shared/GuiPanel_VRTrackingVolumeComponent.h"
#include "TrackingMountComponent.h"
#include "TrackingMountObjectSystem.h"
#include "VRTrackingVolumeComponent.h"
#include "VRTrackingVolumeSystem.h"

#include "imgui.h"

bool GuiPanel_ProjectTracking::init(ProjectGuiPanelContext* context)
{
	m_context = context;
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	m_projectManager = ownerAppStage->getProjectManager();
	m_trackingMountSystem = ownerAppStage->getObjectSystemOfType<TrackingMountObjectSystem>();

	m_defaultGuiStyle = getGuiStyleManager()->getStyle("default_component_panel");

	auto pm = ownerAppStage->getProjectManager();
	m_trackingVolumeDataSource = std::make_unique<GuiDataSource_ComboBox>(pm,
		std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
			{ VRTrackingVolumeSystem::k_objectSystemClassName, VRTrackingVolumeComponent::k_componentClassName },
			{ MarkerTrackingVolumeSystem::k_objectSystemClassName, MarkerTrackingVolumeComponent::k_componentClassName }
		});

	m_trackingMountDataSource = std::make_unique<GuiDataSource_ComboBox>(pm,
		std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
			{ TrackingMountObjectSystem::k_objectSystemClassName, TrackingMountComponent::k_componentClassName }
		});
	m_trackingMountDataSource->setFilter([this](MikanComponentPtr comp) -> bool {
		VRTrackingVolumeComponentPtr vrVolume = getSelectedVRTrackingVolume();
		if (!vrVolume) return false;
		const auto& mountIds = vrVolume->getVRTrackingVolumeDefinition()->getTrackingMountIDs();
		auto id = (MikanTrackingMountID)comp->getComponentId();
		return std::find(mountIds.begin(), mountIds.end(), id) != mountIds.end();
	});

	// Auto-select first tracking volume if available
	m_trackingVolumeDataSource->refreshEntries();
	if (m_trackingVolumeDataSource->getEntryCount() > 0)
	{
		MikanComponentPtr first = m_trackingVolumeDataSource->getEntryAtIndex(0);
		setSelectedTrackingVolumeId((MikanTrackingVolumeID)first->getComponentId());
	}

	// Auto-select first tracking mount for the selected volume
	m_trackingMountDataSource->refreshEntries();
	if (m_trackingMountDataSource->getEntryCount() > 0)
	{
		if (MikanComponentPtr first = m_trackingMountDataSource->getEntryAtIndex(0))
			setSelectedTrackingMountId((MikanTrackingMountID)first->getComponentId());
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

	// Tracking Volumes combo
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

	m_trackingVolumeDataSource->refreshEntries();

	if (m_selectedTrackingVolumeId != INVALID_MIKAN_ID &&
		m_trackingVolumeDataSource->getEntryIndexByComponentId(m_selectedTrackingVolumeId) == -1)
	{
		setSelectedTrackingVolumeId(INVALID_MIKAN_ID);
	}

	int volumeIndex = m_trackingVolumeDataSource->getEntryIndexByComponentId(m_selectedTrackingVolumeId);
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectTrackingVolume", "Tracking Volume",
		m_trackingVolumeDataSource.get(), volumeIndex))
	{
		if (volumeIndex >= 0)
		{
			if (MikanComponentPtr sel = m_trackingVolumeDataSource->getEntryAtIndex(volumeIndex))
			{
				int newId = sel->getComponentId();
				addDeferredGuiEvent([this, newId]() {
					setSelectedTrackingVolumeId((MikanTrackingVolumeID)newId);
				});
			}
		}
	}

	if (m_selectedTrackingVolumeId != INVALID_MIKAN_ID)
	{
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "removeVolume", "delete_component"))
		{
			addDeferredGuiEvent([this]() {
				auto pm = m_projectManager.lock();
				auto vrSys = pm->getSystemOfType<VRTrackingVolumeSystem>();
				auto markerSys = pm->getSystemOfType<MarkerTrackingVolumeSystem>();
				if (m_isVRTrackingVolume)
					vrSys->removeObjectByPrimaryComponentId(m_selectedTrackingVolumeId);
				else
					markerSys->removeObjectByPrimaryComponentId(m_selectedTrackingVolumeId);
			});
		}

		m_context->getMarkerTrackingVolumePanel()->onGui();
		m_context->getVRTrackingVolumePanel()->onGui();
	}

	// Tracking Mounts sub-combo (only for VR volumes)
	if (m_isVRTrackingVolume && m_selectedTrackingVolumeId != INVALID_MIKAN_ID)
	{
		ImGui::Separator();

		TrackingMountObjectSystemPtr mountSystem = getTrackingMountSystem();
		VRTrackingVolumeComponentPtr vrVolume = getSelectedVRTrackingVolume();

		if (vrVolume && mountSystem)
		{
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

			m_trackingMountDataSource->refreshEntries();

			if (m_selectedTrackingMountId != INVALID_MIKAN_ID &&
				m_trackingMountDataSource->getEntryIndexByComponentId(m_selectedTrackingMountId) == -1)
			{
				setSelectedTrackingMountId(INVALID_MIKAN_ID);
			}

			int mountIndex = m_trackingMountDataSource->getEntryIndexByComponentId(m_selectedTrackingMountId);
			if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectTrackingMount", "Tracking Mount",
				m_trackingMountDataSource.get(), mountIndex))
			{
				if (mountIndex >= 0)
				{
					if (MikanComponentPtr sel = m_trackingMountDataSource->getEntryAtIndex(mountIndex))
					{
						int newId = sel->getComponentId();
						addDeferredGuiEvent([this, newId]() {
							setSelectedTrackingMountId((MikanTrackingMountID)newId);
						});
					}
				}
			}

			if (m_selectedTrackingMountId != INVALID_MIKAN_ID)
			{
				ImGui::SameLine();
				if (MkGui::drawImageButton(m_defaultGuiStyle, "removeMount", "delete_component"))
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
