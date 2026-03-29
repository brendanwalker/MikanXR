#include "AppStage.h"
#include "RmlModel_TrackingMountComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_EntityAccessor.h"
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

bool RmlModel_TrackingMountComponent::init(AppStage* ownerAppStage)
{
	m_vrObjectSystem = ownerAppStage->getObjectSystemOfType<VRObjectSystem>();

	return initTypedPropertyInterface<TrackingMountComponent>(ownerAppStage->getRmlContext());
}

bool RmlModel_TrackingMountComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	// Build the list of all VR device paths from the VRObjectSystem
	m_vrDevicePathList->init(
		constructor,
		CommonConfigPtr(),
		"vr_device_paths",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<std::string>& outDevicePathList) {
			auto vrObjectSystem = getVRObjectSystem();
			if (vrObjectSystem)
			{
				const auto& vrDeviceMap = vrObjectSystem->getComponentMap();
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
				outSocketNameList= vrDeviceComponent->getSocketNames();
			}
		});

	constructor.BindEventCallback(
		"select_device_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const Rml::String devicePath = ev.GetParameter<Rml::String>("value", "");
			setDevicePath(devicePath);
		});

	constructor.BindEventCallback(
		"select_socket_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const Rml::String socketName = ev.GetParameter<Rml::String>("value", "");
			setSocketName(socketName);
		});

	return true;
}

bool RmlModel_TrackingMountComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		refreshVRDevicePathList();
		refreshSocketNames();

		return true;
	}

	return false;
}

VRObjectSystemPtr RmlModel_TrackingMountComponent::getVRObjectSystem() const
{
	return m_vrObjectSystem.lock();
}

VRObjectSystemDefinitionPtr RmlModel_TrackingMountComponent::getVRObjectSystemConfig() const
{
	auto vrObjectSystem = getVRObjectSystem();
	if (vrObjectSystem)
	{
		return vrObjectSystem->getTypedDefinition();
	}

	return nullptr;
}

VRDeviceComponentPtr RmlModel_TrackingMountComponent::getVRDeviceComponent() const
{
	TrackingMountComponentPtr trackingMountComponent = getTrackingMountComponent();
	if (trackingMountComponent)
	{
		return trackingMountComponent->getVRDeviceComponent();
	}

	return nullptr;
}

TrackingMountComponentPtr RmlModel_TrackingMountComponent::getTrackingMountComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<TrackingMountComponent>(component);
	}

	return nullptr;
}

void RmlModel_TrackingMountComponent::setDevicePath(const std::string& devicePath)
{
	TrackingMountComponentPtr mountComponent = getTrackingMountComponent();
	if (mountComponent)
	{
		mountComponent->getTrackingMountDefinition()->setDevicePath(devicePath);
	}

	// Clear the current socket now that the device change
	setSocketName("");

	// Rebuild the list of available sockets for the current device
	refreshSocketNames();
}

void RmlModel_TrackingMountComponent::setSocketName(const std::string& socketName)
{
	TrackingMountComponentPtr mountComponent = getTrackingMountComponent();
	if (mountComponent)
	{
		mountComponent->getTrackingMountDefinition()->setSocketName(socketName);
	}
}

void RmlModel_TrackingMountComponent::refreshVRDevicePathList()
{
	m_vrDevicePathList->setOwnerConfig(getVRObjectSystemConfig());
	m_vrDevicePathList->rebuildList(true);
}

void RmlModel_TrackingMountComponent::refreshSocketNames()
{
	TrackingMountComponentPtr trackingMountComponent = getTrackingMountComponent();

	m_socketNameList->setOwnerConfig(trackingMountComponent ? trackingMountComponent->getDefinition() : CommonConfigPtr());
	m_socketNameList->rebuildList(true);
}