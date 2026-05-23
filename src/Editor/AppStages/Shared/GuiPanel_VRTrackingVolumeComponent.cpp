#include "AppStage.h"
#include "MarkerComponent.h"
#include "MikanCoreTypes.h"
#include "Shared/GuiPanel_VRTrackingVolumeComponent.h"
#include "TrackingVolumeComponent.h"
#include "TrackingMountObjectSystem.h"

GuiPanel_VRTrackingVolumeComponent::GuiPanel_VRTrackingVolumeComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_charucoMountDataSource(
		ownerAppStage->getProjectManager(),
		{{ 
				TrackingMountObjectSystem::k_objectSystemClassName,
				TrackingMountComponent::k_componentClassName 
		}})
	, m_originMarkerDataSource(
		ownerAppStage->getProjectManager(),
		{ { MarkerObjectSystem::k_objectSystemClassName, MarkerComponent::k_componentClassName } })
	, m_utilityMarkerDataSource(
		ownerAppStage->getProjectManager(),
		{ { MarkerObjectSystem::k_objectSystemClassName, MarkerComponent::k_componentClassName } })
{
	m_charucoMountDataSource.setFilter([this](MikanComponentPtr comp) -> bool {
		VRTrackingVolumeComponentPtr ownerVolume= getVRTrackingVolumeComponent();
		if (ownerVolume)
		{
			auto trackingMount = std::static_pointer_cast<TrackingMountComponent>(comp);
			return ownerVolume->ownsTrackingMount(trackingMount->getComponentId());
		}
		return false;
	});
}

bool GuiPanel_VRTrackingVolumeComponent::init()
{
	return initTypedPropertyInterface<VRTrackingVolumeComponent>();
}

void GuiPanel_VRTrackingVolumeComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	// Charuco Mount — component combo (property in VRTrackingVolumeDefinition)
	m_entityAccessor->setPropertyRenderer(
		VRTrackingVolumeDefinition::k_charucoMountIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			VRTrackingVolumeComponentPtr vol = getVRTrackingVolumeComponent();
			if (!vol) return false;
			auto volumeDef = vol->getVRTrackingVolumeDefinition();

			m_charucoMountDataSource.refreshEntries();
			const int currentId = volumeDef->getCharucoTrackingMountId();
			int selectedIndex = m_charucoMountDataSource.getEntryIndexByComponentId(currentId);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "charucoMountId", "Charuco Mount",
				&m_charucoMountDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					MikanComponentPtr comp = m_charucoMountDataSource.getEntryAtIndex(selectedIndex);
					if (comp)
					{
						const int newId = comp->getComponentId();
						addDeferredGuiEvent([volumeDef, newId]() {
							volumeDef->setCharucoTrackingMountId(newId);
							});
					}
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
