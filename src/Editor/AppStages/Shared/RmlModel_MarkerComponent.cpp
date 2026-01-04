#include "MarkerComponent.h"
#include "Shared/RmlModel_MarkerComponent.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "Shared/RmlDataBinding_List.h"
#include "MarkerObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_MarkerComponent::RmlModel_MarkerComponent()
	: RmlModel_MikanComponent()
	, m_arucoIdList(std::make_shared<RmlDataBinding_ArucoIdList>())
{}

bool RmlModel_MarkerComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<MarkerComponent>(rmlContext);
}

bool RmlModel_MarkerComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	// Build the list of all aruco IDs by collecting from the marker system config
	m_arucoIdList->init(
		constructor,
		CommonConfigPtr(),
		MarkerObjectSystemConfig::k_arucoIdListPropertyId,
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outArucoIdList) 
		{
			auto markerObjectSystemConfig = getMarkerObjectSystemConfig();
			if (markerObjectSystemConfig)
			{
				markerObjectSystemConfig->getArucoIdList(outArucoIdList);
			}
		});

	constructor.BindEventCallback(
		"select_aruco_id_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int newArucoId = ev.GetParameter<int>("value", 0);
			MarkerComponentPtr markerComponent = getMarkerComponent();
			if (markerComponent)
			{
				markerComponent->getMarkerDefinition()->setArucoId(newArucoId);

				// Notify listeners that a marker was selected
				if (OnMarkerSelected)
				{
					OnMarkerSelected(newArucoId);
				}
			}
		});

	return true;
}

bool RmlModel_MarkerComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_arucoIdList->setOwnerConfig(getMarkerObjectSystemConfig());
		m_arucoIdList->rebuildList(true);

		// Notify listeners that a marker was selected
		MarkerComponentPtr markerComponent = getMarkerComponent();
		if (markerComponent && OnMarkerSelected)
		{
			OnMarkerSelected(markerComponent->getMarkerDefinition()->getArucoId());
		}

		return true;
	}

	return false;
}

MarkerObjectSystemPtr RmlModel_MarkerComponent::getMarkerObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<MarkerObjectSystem>();
	}

	return nullptr;
}

MarkerObjectSystemConfigPtr RmlModel_MarkerComponent::getMarkerObjectSystemConfig() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getMarkerSystemConfig();
	}

	return nullptr;
}

MarkerComponentPtr RmlModel_MarkerComponent::getMarkerComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<MarkerComponent>(component);
	}

	return nullptr;
}