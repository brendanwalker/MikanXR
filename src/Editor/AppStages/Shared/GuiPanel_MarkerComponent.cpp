#include "AppStage.h"
#include "MarkerComponent.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "MarkerObjectSystem.h"

bool GuiPanel_MarkerComponent::init()
{
	m_markerObjectSystem = getOwnerAppStage()->getSystemOfType<MarkerObjectSystem>();
	return initTypedPropertyInterface<MarkerComponent>();
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

void GuiPanel_MarkerComponent::onConstruct()
{
	m_entityAccessor->setPropertyRenderer(
		MarkerDefinition::k_arucoIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			MarkerComponentPtr markerComp = getMarkerComponent();
			if (!markerComp) return false;

			// Build aruco ID list from all markers in the system
			auto markerSystem = getMarkerObjectSystem();
			std::vector<std::string> arucoIdStrings;
			if (markerSystem)
			{
				for (const auto& it : markerSystem->getComponentMap())
				{
					auto markerCompEntry = it.second.lock();
					if (markerCompEntry)
					{
						auto markerTyped = std::static_pointer_cast<MarkerComponent>(markerCompEntry);
						arucoIdStrings.push_back(
							std::to_string(markerTyped->getMarkerDefinition()->getArucoId()));
					}
				}
			}
			m_arucoIdDataSource.setEntries(arucoIdStrings);

			const int currentArucoId = markerComp->getMarkerDefinition()->getArucoId();
			const std::string currentStr = std::to_string(currentArucoId);
			int selectedIndex = m_arucoIdDataSource.getEntryIndexByString(currentStr);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "markerArucoId", "Aruco ID",
				&m_arucoIdDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const int newArucoId =
						std::stoi(m_arucoIdDataSource.getEntryDisplayString(selectedIndex));
					addDeferredGuiEvent([this, markerComp, newArucoId]() {
						markerComp->getMarkerDefinition()->setArucoId(newArucoId);
						if (OnMarkerSelected)
						{
							OnMarkerSelected(newArucoId);
						}
					});
				}
			}
			return true;
		});
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
