#include "RmlModel_StageComponent.h"
#include "TrackingVolumeObjectSystem.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_PropertyInterface.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_StageComponent::RmlModel_StageComponent()
	: RmlModel_MikanComponent()
	, m_trackingVolumeIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_StageComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<StageComponent>(rmlContext);
}

bool RmlModel_StageComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	// Build the list of all tracking volume IDs from the TrackingVolumeObjectSystem
	m_trackingVolumeIdList->init(
		constructor,
		CommonConfigPtr(),
		"tracking_volume_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			auto trackingVolumeObjectSystem= getTrackingVolumeObjectSystem();
			if (trackingVolumeObjectSystem)
			{
				outComponentIdList = trackingVolumeObjectSystem->getTrackingVolumeIdList();
			}
		});

	constructor.BindEventCallback(
		"select_volume_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int newVolumeId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);
			StageComponentPtr stageComponent = getStageComponent();
			if (stageComponent)
			{
				stageComponent->setTrackingVolumeId(newVolumeId);
			}
		});

	return true;
}

bool RmlModel_StageComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_trackingVolumeIdList->setOwnerConfig(getTrackingVolumeObjectSystemConfig());
		m_trackingVolumeIdList->rebuildList(true);

		return true;
	}

	return false;
}

TrackingVolumeObjectSystemPtr RmlModel_StageComponent::getTrackingVolumeObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<TrackingVolumeObjectSystem>();
	}

	return nullptr;
}

TrackingVolumeObjectSystemConfigPtr RmlModel_StageComponent::getTrackingVolumeObjectSystemConfig() const
{
	auto trackingVolumeObjectSystem = getTrackingVolumeObjectSystem();
	if (trackingVolumeObjectSystem)
	{
		return trackingVolumeObjectSystem->getTrackingVolumeSystemConfig();
	}

	return nullptr;
}

StageComponentPtr RmlModel_StageComponent::getStageComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::dynamic_pointer_cast<StageComponent>(component);
	}

	return nullptr;
}