#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "MikanCoreTypes.h"
#include "ProjectConfig.h"
#include "RmlModel_ProjectMarkers.h"
#include "Shared/RmlModel_MarkerComponent.h"
#include "Shared/RmlModel_MarkerObjectSystem.h"
#include "Shared/RmlDataBinding_List.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectMarkers::RmlModel_ProjectMarkers()
	: m_markerIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_selectedMarkerModel(std::make_shared<RmlModel_MarkerComponent>())
	, m_markerSystemModel(std::make_shared<RmlModel_MarkerObjectSystem>())
{
}

bool RmlModel_ProjectMarkers::init(
	Rml::Context* rmlContext, 
	ProjectConfigPtr projectConfig,
	MarkerObjectSystemPtr markerSystem)
{
	MarkerObjectSystemConfigPtr markerSystemConfig = projectConfig->markerSystemConfig;

	m_projectConfig = projectConfig;
	m_markerSystem = markerSystem;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "Markers");
	if (!constructor)
		return false;

	// Register component lists
	m_markerIdList->init(
		constructor, 
		markerSystemConfig,
		"marker_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			MarkerObjectSystemConfigPtr markerConfig = m_projectConfig.lock()->markerSystemConfig;

			for (const auto& markerPtr : markerConfig->getArucoMarkerList())
			{
				if (markerPtr)
				{
					outComponentIdList.push_back((int)markerPtr->getMarkerId());
				}
			}
		});

	// Register Data Model Fields
	constructor.Bind("selected_marker_id", &m_selectedMarkerId);

	// Register Selected Object Models
	m_selectedMarkerModel->init(rmlContext);
	m_markerSystemModel->init(rmlContext);
	m_markerSystemModel->setObjectSystem(markerSystem);

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_marker", &RmlModel_ProjectMarkers::addNewMarker, this);
	constructor.BindEventCallback("remove_marker", &RmlModel_ProjectMarkers::removeMarker, this);
	constructor.BindEventCallback("select_marker_entry", &RmlModel_ProjectMarkers::selectMarkerEntry, this);

	// Listen for marker config changes
	m_markerIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectMarkers::markerIdListChanged);

	// Forward marker selection events from the marker component model
	m_selectedMarkerModel->OnMarkerSelected = MakeDelegate(this, &RmlModel_ProjectMarkers::onMarkerSelectedFromComponent);

	return true;
}

void RmlModel_ProjectMarkers::dispose()
{
	m_markerIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectMarkers::markerIdListChanged);
	m_selectedMarkerModel->OnMarkerSelected.Clear();

	m_selectedMarkerModel->dispose();
	m_markerSystemModel->dispose();

	RmlModel::dispose();
}

void RmlModel_ProjectMarkers::markerIdListChanged(bool bOwnerChanged)
{
	MikanMarkerID selectedMarkerId = INVALID_MIKAN_ID;
	if (!m_markerIdList->isEmpty() &&
		!m_markerIdList->contains(m_selectedMarkerId))
	{
		selectedMarkerId = m_markerIdList->getFirstValue();
	}

	// Defer the selection update to post view update after element list refreshes
	addModelUpdateCallback([this, selectedMarkerId]() {
		setSelectedMarkerId(selectedMarkerId);
	});
}

MarkerObjectSystemPtr RmlModel_ProjectMarkers::getMarkerSystem()
{
	return m_markerSystem.lock();
}

MarkerComponentPtr RmlModel_ProjectMarkers::getSelectedMarker()
{
	return getMarkerSystem()->getMarkerById((MikanMarkerID)m_selectedMarkerId);
}

void RmlModel_ProjectMarkers::addNewMarker(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getMarkerSystem()->addNewMarker();
}

void RmlModel_ProjectMarkers::removeMarker(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int markerId = parameters[0].Get<int>();
	
	getMarkerSystem()->removeMarker((MikanMarkerID)markerId);
}

void RmlModel_ProjectMarkers::selectMarkerEntry(
	Rml::DataModelHandle handle,
	Rml::Event& ev,
	const Rml::VariantList& parameters)
{
	const int selectedMarkerId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

	setSelectedMarkerId(selectedMarkerId);
}

void RmlModel_ProjectMarkers::setSelectedMarkerId(MikanMarkerID markerId)
{
	if (markerId != m_selectedMarkerId)
	{
		m_selectedMarkerId = (int)markerId;
		m_modelHandle.DirtyVariable("selected_marker_id");

		if (MarkerComponentPtr markerComponent = getSelectedMarker())
		{
			m_selectedMarkerModel->setComponent(markerComponent);
		}
		else
		{
			m_selectedMarkerModel->setComponent(nullptr);
		}
	}
}

void RmlModel_ProjectMarkers::onMarkerSelectedFromComponent(int arucoId)
{
	// Forward the event to listeners of this model
	if (OnMarkerSelected)
	{
		OnMarkerSelected(arucoId);
	}
}