#include "AppStage.h"
#include "IMkGraphicsContext.h"
#include "IMkTexture.h"
#include "MarkerComponent.h"
#include "Shared/GuiDataSource_IntList.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "MarkerObjectSystem.h"

bool GuiPanel_MarkerComponent::init()
{
	m_markerObjectSystem= getOwnerAppStage()->getSystemOfType<MarkerObjectSystem>();
	return initTypedPropertyInterface<MarkerComponent>();
}

bool GuiPanel_MarkerComponent::setComponent(MikanComponentPtr component)
{
	if (GuiPanel_MikanComponent::setComponent(component))
	{
		MarkerComponentPtr markerComp= getMarkerComponent();
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
	GuiPanel_MikanComponent::onConstruct();

	m_entityAccessor->setPropertyRenderer(
		MarkerDefinition::k_arucoIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			MarkerComponentPtr markerComp= getMarkerComponent();
			if (!markerComp)
				return false;

			// Build aruco ID list from all markers in the system
			std::vector<int> markerIdList;
			getMarkerObjectSystem()->getTypedDefinition()->getArucoIdList(markerIdList);
			m_arucoIdDataSource.setEntries(markerIdList);

			const int currentArucoId= markerComp->getMarkerDefinition()->getArucoId();
			int selectedIndex= m_arucoIdDataSource.getEntryIndexByValue(currentArucoId);

			if (MkGui::drawComboBoxProperty(m_defaultGuiStyle,
											markerComp->makePropertyUIIdentifier(MarkerDefinition::k_arucoIdPropertyId),
											"Aruco ID", &m_arucoIdDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const int newArucoId= m_arucoIdDataSource.getEntryValue(selectedIndex);
					addDeferredGuiEvent(
						[this, markerComp, newArucoId]()
						{
							markerComp->getMarkerDefinition()->setArucoId(newArucoId);
							if (OnMarkerSelected)
							{
								OnMarkerSelected(newArucoId);
							}
						});
				}
			}
			// Display the marker preview (texture cached on MarkerComponent)
			IMkGraphicsContext* graphicsContext= getOwnerAppStage()->getGraphicsContext();
			IMkTexturePtr markerTexture= markerComp->getMarkerTexture(graphicsContext);
			if (markerTexture)
			{
				MkGui::drawImage(markerTexture, 128.0f, 128.0f);
			}

			return true;
		});
}

MarkerObjectSystemPtr GuiPanel_MarkerComponent::getMarkerObjectSystem() const { return m_markerObjectSystem.lock(); }

MarkerObjectSystemDefinitionPtr GuiPanel_MarkerComponent::getMarkerObjectSystemDefinition() const
{
	auto markerObjectSystem= getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getTypedDefinition();
	}
	return nullptr;
}

MarkerComponentPtr GuiPanel_MarkerComponent::getMarkerComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<MarkerComponent>(component);
	}
	return nullptr;
}
