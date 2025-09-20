#include "TrackingMountComponent.h"
#include "RmlModel_TrackingMountComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "VRObjectSystem.h"
#include "VRDeviceComponent.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_TrackingMountComponent::RmlModel_TrackingMountComponent()
	: RmlModel_MikanComponent()
	, m_vrDevicePathList(std::make_shared<RmlDataBinding_VRDevicePathList>())
	, m_socketNameList(std::make_shared<RmlDataBinding_SocketNameList>())
{}

bool RmlModel_TrackingMountComponent::init(Rml::Context* rmlContext)
{
	bool bSuccess=
		m_propertyInterface->init<TrackingMountComponent>(
			rmlContext,
			"TrackingMountComponent",
			[this](Rml::DataModelConstructor& constructor) -> bool {

				// Build the list of all VR device paths from the VRObjectSystem
				m_vrDevicePathList->init(
					constructor,
					CommonConfigPtr(),
					"vr_device_paths",
					[this](CommonConfigPtr ownerConfig, Rml::Vector<std::string>& outDevicePathList) {
						auto vrObjectSystem = getVRObjectSystem();
						if (vrObjectSystem)
						{
							const auto& vrDeviceMap = vrObjectSystem->getVRDeviceMap();
							for (const auto& it : vrDeviceMap)
							{
								if (auto vrDeviceComponent = it.second.lock())
								{
									outDevicePathList.push_back(vrDeviceComponent->getVRDeviceDefinition()->getVRDevicePath());
								}
							}
						}
					});

				// Build the list of socket names from the currently selected TrackingMount's VRDeviceComponent
				m_socketNameList->init(
					constructor,
					CommonConfigPtr(),
					"socket_names",
					[this](CommonConfigPtr ownerConfig, Rml::Vector<std::string>& outSocketNameList) {
						auto vrDeviceComponent = getVRDeviceComponent();
						if (vrDeviceComponent)
						{
							vrDeviceComponent->getSocketNames(outSocketNameList);
						}
					});

				return true;
			});

	return true;
}

bool RmlModel_TrackingMountComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_vrDevicePathList->setOwnerConfig(getVRObjectSystemConfig());
		m_vrDevicePathList->rebuildList(true);

		m_socketNameList->setOwnerConfig(component ? component->getDefinition() : CommonConfigPtr());
		m_socketNameList->rebuildList(true);

		return true;
	}

	return false;
}

VRObjectSystemPtr RmlModel_TrackingMountComponent::getVRObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<VRObjectSystem>();
	}

	return nullptr;
}

VRObjectSystemConfigPtr RmlModel_TrackingMountComponent::getVRObjectSystemConfig() const
{
	auto vrObjectSystem = getVRObjectSystem();
	if (vrObjectSystem)
	{
		return vrObjectSystem->getVRSystemConfig();
	}

	return nullptr;
}

VRDeviceComponentPtr RmlModel_TrackingMountComponent::getVRDeviceComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		auto trackingMountComponent = std::static_pointer_cast<TrackingMountComponent>(component);
		if (trackingMountComponent)
		{
			return trackingMountComponent->getVRDeviceComponent();
		}
	}

	return nullptr;
}