#include "GuiPanel_ProjectMarkers.h"
#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "MikanCoreTypes.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "Shared/GuiPanel_MarkerObjectSystem.h"

#include "imgui.h"

bool GuiPanel_ProjectMarkers::init(ProjectGuiPanelContext* context)
{
	m_context= context;
	AppStage_Project* ownerAppStage= context->getOwnerAppStage();
	m_markerSystem= ownerAppStage->getObjectSystemOfType<MarkerObjectSystem>();

	m_defaultGuiStyle= getGuiStyleManager()->getStyle("default_component_panel");

	auto pm= ownerAppStage->getProjectManager();
	m_markerDataSource= std::make_unique<GuiDataSource_ComboBox>(
		pm, std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
				{MarkerObjectSystem::k_objectSystemClassName, MarkerComponent::k_componentClassName}});
	m_markerDataSource->setDisplayStringBuilder(
		[](MikanComponentPtr comp) -> std::string
		{
			auto marker= std::static_pointer_cast<MarkerComponent>(comp);
			std::string name= comp->getName().empty()
								  ? locFormat("project.markerFallbackNameFmt", comp->getComponentId())
								  : comp->getName();
			return name + locFormat("project.markerArucoSuffixFmt", marker->getMarkerDefinition()->getArucoId());
		});

	// Auto-select first marker if available
	MarkerObjectSystemPtr markerSystem= getMarkerSystem();
	if (markerSystem && !markerSystem->getComponentMap().empty())
	{
		MikanMarkerID firstId= (MikanMarkerID)markerSystem->getComponentMap().begin()->first;
		setSelectedMarkerId(firstId);
	}

	return true;
}

void GuiPanel_ProjectMarkers::dispose() { GuiPanel::dispose(); }

MarkerObjectSystemPtr GuiPanel_ProjectMarkers::getMarkerSystem() const { return m_markerSystem.lock(); }

MarkerComponentPtr GuiPanel_ProjectMarkers::getSelectedMarker() const
{
	MarkerObjectSystemPtr markerSystem= getMarkerSystem();
	if (markerSystem && m_selectedMarkerId != INVALID_MIKAN_ID)
		return markerSystem->getMarkerById((MikanMarkerID)m_selectedMarkerId);
	return nullptr;
}

void GuiPanel_ProjectMarkers::setSelectedMarkerId(MikanMarkerID markerId)
{
	m_selectedMarkerId= (int)markerId;

	MarkerObjectSystemPtr markerSystem= getMarkerSystem();
	if (markerSystem)
	{
		MarkerComponentPtr markerComponent= markerSystem->getMarkerById(markerId);
		m_context->getMarkerPanel()->setComponent(markerComponent);
	}
}

void GuiPanel_ProjectMarkers::onGui()
{
	MarkerObjectSystemPtr markerSystem= getMarkerSystem();
	if (!markerSystem)
		return;

	// Markers list
	m_markerDataSource->refreshEntries();

	// Validate selection
	if (m_selectedMarkerId != INVALID_MIKAN_ID
		&& m_markerDataSource->getEntryIndexByComponentId(m_selectedMarkerId) == -1)
	{
		setSelectedMarkerId(INVALID_MIKAN_ID);
	}

	int markerIndex= m_markerDataSource->getEntryIndexByComponentId(m_selectedMarkerId);
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectMarker", locText("project.marker"),
									m_markerDataSource.get(), markerIndex))
	{
		if (markerIndex >= 0)
		{
			if (MikanComponentPtr sel= m_markerDataSource->getEntryAtIndex(markerIndex))
			{
				int newId= sel->getComponentId();
				addDeferredGuiEvent([this, newId]() { setSelectedMarkerId((MikanMarkerID)newId); });
			}
		}
	}

	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addMarker", "add_component"))
	{
		addDeferredGuiEvent(
			[this]()
			{
				MarkerObjectSystemPtr sys= getMarkerSystem();
				if (sys)
				{
					MarkerComponentPtr marker= sys->addNewObjectByTypedDefinition();
					MikanMarkerID markerId= marker->getComponentId();
					setSelectedMarkerId(markerId);
				}
			});
	}

	if (m_selectedMarkerId != INVALID_MIKAN_ID)
	{
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "removeMarker", "delete_component"))
		{
			addDeferredGuiEvent(
				[this]()
				{
					MarkerObjectSystemPtr sys= getMarkerSystem();
					if (sys)
						sys->removeObjectByPrimaryComponentId(m_selectedMarkerId);
				});
		}

		// Show selected marker component
		m_context->getMarkerPanel()->onGui();
	}

	// Show the marker system settings
	m_context->getMarkerSystemPanel()->onGui();
}
