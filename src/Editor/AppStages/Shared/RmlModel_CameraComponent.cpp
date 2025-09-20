#include "CameraComponent.h"
#include "RmlModel_CameraComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "VRTrackingVolumeComponent.h"
#include "VideoSourceSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_CameraComponent::RmlModel_CameraComponent()
	: RmlModel_MikanComponent() 
	, m_trackingMountIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_videoSourceIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_CameraComponent::init(Rml::Context* rmlContext)
{
	bool bSuccess= 
		m_propertyInterface->init<CameraComponent>(
			rmlContext, 
			"CameraComponent",
			[this](Rml::DataModelConstructor& constructor) -> bool {

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

				// Build the list of all video source IDs from the VideoSourceSystem
				m_videoSourceIdList->init(
					constructor,
					CommonConfigPtr(),
					"video_source_ids",
					[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
						auto videoSourceSystem= getVideoSourceSystem();
						if (videoSourceSystem)
						{
							outComponentIdList = videoSourceSystem->getVideoSourceIdList();
						}
					});

				return true;
			});

	return true;
}

bool RmlModel_CameraComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_trackingMountIdList->setOwnerConfig(getOwnerVRTrackingVolume());
		m_trackingMountIdList->rebuildList(true);

		m_videoSourceIdList->setOwnerConfig(getVideoSourceSystemConfig());
		m_videoSourceIdList->rebuildList(true);

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

VideoSourceSystemPtr RmlModel_CameraComponent::getVideoSourceSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<VideoSourceSystem>();
	}

	return nullptr;
}

VideoSourceSystemConfigPtr RmlModel_CameraComponent::getVideoSourceSystemConfig() const
{
	auto videoSourceSystem = getVideoSourceSystem();
	if (videoSourceSystem)
	{
		return videoSourceSystem->getVideoSourceSystemConfig();
	}

	return nullptr;
}