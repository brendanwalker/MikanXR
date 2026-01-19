#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "MikanCoreTypes.h"
#include "ProjectConfig.h"
#include "RmlModel_ProjectMarkers.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectRmlModelContext.h"
#include "Shared/RmlModel_MarkerComponent.h"
#include "Shared/RmlModel_MarkerObjectSystem.h"
#include "Shared/RmlDataBinding_List.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectMarkers::RmlModel_ProjectMarkers()
	: m_markerIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{
}

bool RmlModel_ProjectMarkers::init(
	ProjectRmlModelContext* context)
{
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	Rml::Context* rmlContext = ownerAppStage->getRmlContext();

	m_projectRmlModelContext = context;
	m_markerSystem = ownerAppStage->getObjectSystemOfType<MarkerObjectSystem>();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "Markers");
	if (!constructor)
		return false;

	// Register component lists
	m_markerIdList->init(
		constructor,
		m_markerSystem.lock(),
		MarkerObjectSystemDefinition::k_componentIdListPropertyId);

	// Register Data Model Fields
	constructor.Bind("selected_marker_id", &m_selectedMarkerId);

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_marker", &RmlModel_ProjectMarkers::addNewMarker, this);
	constructor.BindEventCallback("remove_marker", &RmlModel_ProjectMarkers::removeMarker, this);
	constructor.BindEventCallback("select_marker_entry", &RmlModel_ProjectMarkers::selectMarkerEntry, this);

	// Listen for marker config changes
	m_markerIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectMarkers::markerIdListChanged);

	// Auto-select first items if available
	if (!m_markerIdList->isEmpty())
	{
		setSelectedMarkerId(m_markerIdList->getFirstValue());
	}

	return true;
}

void RmlModel_ProjectMarkers::dispose()
{
	m_markerIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectMarkers::markerIdListChanged);

	RmlModel::dispose();
}

void RmlModel_ProjectMarkers::markerIdListChanged(bool bOwnerChanged)
{
	if (MikanMarkerID newSelectedMarkerId = m_selectedMarkerId;
		m_markerIdList->fixupSelectedIndex(m_selectedMarkerId, newSelectedMarkerId))
	{
		// Defer the selection update to post view update after element list refreshes
		addModelUpdateCallback([this, newSelectedMarkerId]() {
			setSelectedMarkerId(newSelectedMarkerId);
		});
	}
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
	MarkerComponentPtr markerComponent= getMarkerSystem()->addNewObject();
	MikanMarkerID markerId = markerComponent->getMarkerDefinition()->getMarkerId();

	addModelUpdateCallback([this, markerId]() {
		setSelectedMarkerId(markerId);
	});
}

void RmlModel_ProjectMarkers::removeMarker(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int markerId = parameters[0].Get<int>();

	getMarkerSystem()->removeObject((MikanMarkerID)markerId);
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
			m_projectRmlModelContext->getMarkerModel()->setComponent(markerComponent);
		}
		else
		{
			m_projectRmlModelContext->getMarkerModel()->setComponent(nullptr);
		}
	}
}