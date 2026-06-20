#include "AppStage.h"
#include "MarkerComponent.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MikanCoreTypes.h"
#include "Shared/GuiPanel_MarkerTrackingVolumeComponent.h"
#include "TrackingVolumeComponent.h"

GuiPanel_MarkerTrackingVolumeComponent::GuiPanel_MarkerTrackingVolumeComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_originMarkerDataSource(
		  ownerAppStage->getProjectManager(),
		  {{MarkerObjectSystem::k_objectSystemClassName, MarkerComponent::k_componentClassName}})
{
}

bool GuiPanel_MarkerTrackingVolumeComponent::init()
{
	return initTypedPropertyInterface<MarkerTrackingVolumeComponent>();
}

void GuiPanel_MarkerTrackingVolumeComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	// Origin Marker — component combo (property in TrackingVolumeDefinition base class)
	m_entityAccessor->setPropertyRenderer(
		TrackingVolumeDefinition::k_originMarkerIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			MarkerTrackingVolumeComponentPtr volumeComp= getMarkerTrackingVolumeComponent();
			if (!volumeComp)
				return false;
			auto volumeDef= volumeComp->getMarkerTrackingVolumeDefinition();

			m_originMarkerDataSource.refreshEntries();
			const int currentId= volumeDef->getOriginMarkerId();
			int selectedIndex= m_originMarkerDataSource.getEntryIndexByComponentId(currentId);

			if (MkGui::drawComboBoxProperty(
					m_defaultGuiStyle,
					volumeComp->makePropertyUIIdentifier(TrackingVolumeDefinition::k_originMarkerIdPropertyId),
					"Origin Marker",
					&m_originMarkerDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					MikanComponentPtr comp= m_originMarkerDataSource.getEntryAtIndex(selectedIndex);
					if (comp)
					{
						const int newId= comp->getComponentId();
						addDeferredGuiEvent([volumeComp, newId]()
											{ volumeComp->getMarkerTrackingVolumeDefinition()->setOriginMarkerId(newId); });
					}
				}
			}
			return true;
		});
}

MarkerTrackingVolumeComponentPtr GuiPanel_MarkerTrackingVolumeComponent::getMarkerTrackingVolumeComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::dynamic_pointer_cast<MarkerTrackingVolumeComponent>(component);
	}
	return nullptr;
}
