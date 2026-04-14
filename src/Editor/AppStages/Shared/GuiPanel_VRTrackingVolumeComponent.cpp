#include "AppStage.h"
#include "MarkerComponent.h"
#include "MikanCoreTypes.h"
#include "Shared/GuiPanel_VRTrackingVolumeComponent.h"
#include "TrackingVolumeComponent.h"

GuiPanel_VRTrackingVolumeComponent::GuiPanel_VRTrackingVolumeComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_originMarkerDataSource(
		ownerAppStage->getProjectManager(),
		{ { MarkerObjectSystem::k_objectSystemClassName, MarkerComponent::k_componentClassName } })
	, m_utilityMarkerDataSource(
		ownerAppStage->getProjectManager(),
		{ { MarkerObjectSystem::k_objectSystemClassName, MarkerComponent::k_componentClassName } })
{
}

bool GuiPanel_VRTrackingVolumeComponent::init()
{
	return initTypedPropertyInterface<VRTrackingVolumeComponent>();
}

void GuiPanel_VRTrackingVolumeComponent::onConstruct()
{
	// Charuco Mount — string list built from this volume's mount IDs
	m_entityAccessor->setPropertyRenderer(
		VRTrackingVolumeDefinition::k_charucoMountIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			VRTrackingVolumeComponentPtr vol = getVRTrackingVolumeComponent();
			if (!vol) return false;
			auto volumeDef = vol->getVRTrackingVolumeDefinition();

			std::vector<std::string> mountIdStrings;
			for (int id : volumeDef->getTrackingMountIDs())
				mountIdStrings.push_back(std::to_string(id));
			m_charucoMountDataSource.setEntries(mountIdStrings);

			const int currentId = volumeDef->getCharucoTrackingMountId();
			const std::string currentStr =
				(currentId == INVALID_MIKAN_ID) ? "" : std::to_string(currentId);
			int selectedIndex = m_charucoMountDataSource.getEntryIndexByString(currentStr);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "charucoMountId", "Charuco Mount",
				&m_charucoMountDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const int newId =
						std::stoi(m_charucoMountDataSource.getEntryDisplayString(selectedIndex));
					addDeferredGuiEvent([volumeDef, newId]() {
						volumeDef->setCharucoTrackingMountId(newId);
					});
				}
			}
			return true;
		});

	// Origin Marker — component combo (property in TrackingVolumeDefinition base class)
	m_entityAccessor->setPropertyRenderer(
		TrackingVolumeDefinition::k_originMarkerIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			VRTrackingVolumeComponentPtr vol = getVRTrackingVolumeComponent();
			if (!vol) return false;
			auto volumeDef = vol->getVRTrackingVolumeDefinition();

			m_originMarkerDataSource.refreshEntries();
			const int currentId = volumeDef->getOriginMarkerId();
			int selectedIndex = m_originMarkerDataSource.getEntryIndexByComponentId(currentId);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "originMarkerId", "Origin Marker",
				&m_originMarkerDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					MikanComponentPtr comp = m_originMarkerDataSource.getEntryAtIndex(selectedIndex);
					if (comp)
					{
						const int newId = comp->getComponentId();
						addDeferredGuiEvent([volumeDef, newId]() {
							volumeDef->setOriginMarkerId(newId);
						});
					}
				}
			}
			return true;
		});

	// Utility Marker — component combo
	m_entityAccessor->setPropertyRenderer(
		VRTrackingVolumeDefinition::k_utilityMarkerIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			VRTrackingVolumeComponentPtr vol = getVRTrackingVolumeComponent();
			if (!vol) return false;
			auto volumeDef = vol->getVRTrackingVolumeDefinition();

			m_utilityMarkerDataSource.refreshEntries();
			const int currentId = volumeDef->getUtilityMarkerId();
			int selectedIndex = m_utilityMarkerDataSource.getEntryIndexByComponentId(currentId);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "utilityMarkerId", "Utility Marker",
				&m_utilityMarkerDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					MikanComponentPtr comp = m_utilityMarkerDataSource.getEntryAtIndex(selectedIndex);
					if (comp)
					{
						const int newId = comp->getComponentId();
						addDeferredGuiEvent([volumeDef, newId]() {
							volumeDef->setUtilityMarkerId(newId);
						});
					}
				}
			}
			return true;
		});
}

void GuiPanel_VRTrackingVolumeComponent::onGui()
{
	GuiPanel_MikanComponent::onGui();
}

VRTrackingVolumeComponentPtr GuiPanel_VRTrackingVolumeComponent::getVRTrackingVolumeComponent() const
{
	return std::dynamic_pointer_cast<VRTrackingVolumeComponent>(m_component.lock());
}
