#include "AppStage.h"
#include "RmlModel_CameraComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "ProjectManager.h"
#include "VRTrackingVolumeComponent.h"
#include "VideoSourceQueries.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_CameraComponent::RmlModel_CameraComponent()
	: RmlModel_MikanComponent()
	, m_trackingMountIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_textureSourceIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_CameraComponent::init(class AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<CameraComponent>(ownerAppStage->getRmlContext());
}

bool RmlModel_CameraComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	// Build the list of tracking mount IDs from the associated VRTrackingVolumeDefinition
	m_trackingMountIdList->init(
		constructor,
		CommonConfigPtr(),
		"tracking_mount_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			if (ownerConfig)
			{
				auto vrVolumeConfig = std::static_pointer_cast<VRTrackingVolumeDefinition>(ownerConfig);

				outComponentIdList = vrVolumeConfig->getTrackingMountIDs();
			}
		});

	// Build the list of all video source IDs from the VideoSourceQueries
	m_textureSourceIdList->init(
		constructor,
		CommonConfigPtr(),
		"video_source_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			MikanComponentPtr component = m_component.lock();
			if (component)
			{
				ProjectManagerPtr projectManager = component->getOwnerProjectManager();
				outComponentIdList = VideoSourceQueries::getVideoSourceIdList(projectManager);
			}
		});

	constructor.BindEventCallback(
		"select_video_source_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int selectedVideoSourceId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

			CameraComponentPtr cameraComponent= getCameraComponent();
			if (cameraComponent)
				cameraComponent->getCameraDefinition()->setVideoSourceId(selectedVideoSourceId);
		});
	constructor.BindEventCallback(
		"select_mount_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int selectedMountId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

			CameraComponentPtr cameraComponent = getCameraComponent();
			if (cameraComponent)
				cameraComponent->getCameraDefinition()->setTrackingMountId(selectedMountId);
		});

	return true;
}

bool RmlModel_CameraComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_trackingMountIdList->setOwnerConfig(getOwnerVRTrackingVolume());
		m_trackingMountIdList->rebuildList(true);

		m_textureSourceIdList->setOwnerConfig(CommonConfigPtr());
		m_textureSourceIdList->rebuildList(true);

		return true;
	}

	return false;
}

VRTrackingVolumeDefinitionPtr RmlModel_CameraComponent::getOwnerVRTrackingVolume() const
{
	MikanComponentPtr component = m_component.lock();

	if (component)
	{
		auto cameraComponent = std::static_pointer_cast<CameraComponent>(component);
		return cameraComponent->getVRTrackingVolumeDefinitionMutable();
	}

	return nullptr;
}

CameraComponentPtr RmlModel_CameraComponent::getCameraComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<CameraComponent>(component);
	}
	return nullptr;
}