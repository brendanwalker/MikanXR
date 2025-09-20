#include "CompositorComponent.h"
#include "Shared/RmlModel_CompositorComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "CameraObjectSystem.h"
#include "VideoSourceSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_CompositorComponent::RmlModel_CompositorComponent()
	: RmlModel_MikanComponent()
	, m_cameraIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_videoSourceIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_CompositorComponent::init(Rml::Context* rmlContext)
{
	bool bSuccess=
		m_propertyInterface->init<CompositorComponent>(
			rmlContext,
			"CompositorComponent",
			[this](Rml::DataModelConstructor& constructor) -> bool {

				// Build the list of all camera IDs from the CameraObjectSystem
				m_cameraIdList->init(
					constructor,
					CommonConfigPtr(),
					"camera_ids",
					[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
						auto cameraObjectSystem= getCameraObjectSystem();
						if (cameraObjectSystem)
						{
							outComponentIdList = cameraObjectSystem->getAllCameraIds();
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

bool RmlModel_CompositorComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_cameraIdList->setOwnerConfig(getCameraObjectSystemConfig());
		m_cameraIdList->rebuildList(true);

		m_videoSourceIdList->setOwnerConfig(getVideoSourceSystemConfig());
		m_videoSourceIdList->rebuildList(true);

		return true;
	}

	return false;
}

CameraObjectSystemPtr RmlModel_CompositorComponent::getCameraObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<CameraObjectSystem>();
	}

	return nullptr;
}

CameraObjectSystemConfigPtr RmlModel_CompositorComponent::getCameraObjectSystemConfig() const
{
	auto cameraObjectSystem = getCameraObjectSystem();
	if (cameraObjectSystem)
	{
		return cameraObjectSystem->getCameraSystemConfig();
	}

	return nullptr;
}

VideoSourceSystemPtr RmlModel_CompositorComponent::getVideoSourceSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<VideoSourceSystem>();
	}

	return nullptr;
}

VideoSourceSystemConfigPtr RmlModel_CompositorComponent::getVideoSourceSystemConfig() const
{
	auto videoSourceSystem = getVideoSourceSystem();
	if (videoSourceSystem)
	{
		return videoSourceSystem->getVideoSourceSystemConfig();
	}

	return nullptr;
}