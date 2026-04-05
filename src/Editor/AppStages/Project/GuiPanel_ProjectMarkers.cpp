#include "GuiPanel_ProjectMarkers.h"
#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "MikanCoreTypes.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "Shared/GuiPanel_MarkerObjectSystem.h"
#include "StringUtils.h"

#include "imgui.h"

bool GuiPanel_ProjectMarkers::init(ProjectGuiPanelContext* context)
{
	m_context = context;
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	m_markerSystem = ownerAppStage->getObjectSystemOfType<MarkerObjectSystem>();

	// Auto-select first marker if available
	MarkerObjectSystemPtr markerSystem = getMarkerSystem();
	if (markerSystem && !markerSystem->getComponentMap().empty())
	{
		MikanMarkerID firstId = (MikanMarkerID)markerSystem->getComponentMap().begin()->first;
		setSelectedMarkerId(firstId);
	}

	return true;
}

void GuiPanel_ProjectMarkers::dispose()
{
	GuiPanel::dispose();
}

MarkerObjectSystemPtr GuiPanel_ProjectMarkers::getMarkerSystem() const
{
	return m_markerSystem.lock();
}

MarkerComponentPtr GuiPanel_ProjectMarkers::getSelectedMarker() const
{
	MarkerObjectSystemPtr markerSystem = getMarkerSystem();
	if (markerSystem && m_selectedMarkerId != INVALID_MIKAN_ID)
		return markerSystem->getMarkerById((MikanMarkerID)m_selectedMarkerId);
	return nullptr;
}

void GuiPanel_ProjectMarkers::setSelectedMarkerId(MikanMarkerID markerId)
{
	m_selectedMarkerId = (int)markerId;

	MarkerObjectSystemPtr markerSystem = getMarkerSystem();
	if (markerSystem)
	{
		MarkerComponentPtr markerComponent = markerSystem->getMarkerById(markerId);
		m_context->getMarkerPanel()->setComponent(markerComponent);
	}
}

void GuiPanel_ProjectMarkers::onGui()
{
	MarkerObjectSystemPtr markerSystem = getMarkerSystem();
	if (!markerSystem)
		return;

	const auto& componentMap = markerSystem->getComponentMap();

	// Validate selection
	if (m_selectedMarkerId != INVALID_MIKAN_ID &&
		componentMap.find(m_selectedMarkerId) == componentMap.end())
	{
		setSelectedMarkerId(INVALID_MIKAN_ID);
	}

	// Markers list
	if (ImGui::Button("Add Marker"))
	{
		addDeferredGuiEvent([this]() {
			MarkerObjectSystemPtr sys = getMarkerSystem();
			if (sys)
			{
				MarkerComponentPtr marker = sys->addNewObjectByTypedDefinition();
				MikanMarkerID markerId = marker->getComponentId();
				setSelectedMarkerId(markerId);
			}
			});
	}

	if (ImGui::BeginListBox("Markers", ImVec2(-1, 120)))
	{
		for (const auto& [id, weakPtr] : componentMap)
		{
			MarkerComponentPtr marker = std::static_pointer_cast<MarkerComponent>(weakPtr.lock());
			if (!marker)
				continue;

			auto markerDefinition = marker->getMarkerDefinition();
			std::string label = marker->getName().empty()
				? ("Marker " + std::to_string(id))
				: marker->getName();
			label += " (Aruco:" + std::to_string(markerDefinition->getArucoId()) + ")##" + std::to_string(id);

			bool selected = (m_selectedMarkerId == (int)id);
			if (ImGui::Selectable(label.c_str(), selected))
			{
				int newId = (int)id;
				addDeferredGuiEvent([this, newId]() {
					setSelectedMarkerId((MikanMarkerID)newId);
				});
			}
		}
		ImGui::EndListBox();
	}

	if (m_selectedMarkerId != INVALID_MIKAN_ID)
	{
		if (ImGui::Button("Remove Marker"))
		{
			addDeferredGuiEvent([this]() {
				MarkerObjectSystemPtr sys = getMarkerSystem();
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
