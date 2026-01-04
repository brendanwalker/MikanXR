#include "RmlModel_VideoSourceComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "NetworkVideoSourceComponent.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceSystem.h"
#include "VideoSourceSettings/RmlDataBinding_VideoSourceSetting.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

// -- RmlModel_NetworkVideoSourceComponent -----
bool RmlModel_NetworkVideoSourceComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<NetworkVideoSourceComponent>(rmlContext);
}

bool RmlModel_NetworkVideoSourceComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	constructor.BindEventCallback(
		"select_protocol",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto newProtocol = ev.GetParameter<Rml::String>("value", "");
			auto videoSourceComponent = getNetworkVideoSourceComponent();
			if (videoSourceComponent)
			{
				videoSourceComponent->getNetworkVideoSourceDefinition()->setProtocol(newProtocol);
			}
		});

	return true;
}

NetworkVideoSourceComponentPtr RmlModel_NetworkVideoSourceComponent::getNetworkVideoSourceComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<NetworkVideoSourceComponent>(component);
	}

	return nullptr;
}

// -- RmlModel_USBVideoSourceComponent -----
RmlModel_USBVideoSourceComponent::RmlModel_USBVideoSourceComponent()
	: RmlModel_MikanComponent()
	, m_usbDevicePathList(std::make_shared<RmlDataBinding_USBDevicePathList>())
	, m_videoModeNameList(std::make_shared<RmlDataBinding_VideoModeList>())
{
	for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
	{
		m_videoSourceSettings[settingIndex] =
			std::make_shared<RmlDataBinding_VideoSourceSetting>(
				static_cast<eVideoSettingType>(settingIndex));
	}
}

bool RmlModel_USBVideoSourceComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<USBVideoSourceComponent>(rmlContext);
}

bool RmlModel_USBVideoSourceComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	// Register Video Setting Data Bindings
	for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
	{
		auto& videoSettingBinding = m_videoSourceSettings[settingIndex];
		if (!videoSettingBinding->init(constructor))
		{
			return false;
		}
	}

	// Build the list of all usb device paths from the USBVideoSourceSystem
	m_usbDevicePathList->init(
		constructor,
		CommonConfigPtr(),
		"usb_device_paths",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<std::string>& outDevicePathList) {
			auto usbVideoSourceSystem = getUSBVideoSourceSystem();
			if (usbVideoSourceSystem)
			{
				usbVideoSourceSystem->getConnectedUSBVideoSourcePaths(outDevicePathList);
			}
		});

	// Build the list of video modes from the currently selected USBVideoSourceComponent
	m_videoModeNameList->init(
		constructor,
		CommonConfigPtr(),
		"video_modes",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<std::string>& outVideoModeList) {
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				videoSourceComponent->getVideoModeNames(outVideoModeList);
			}
		});

	constructor.BindEventCallback(
		"select_video_device_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto newDevicePath = ev.GetParameter<Rml::String>("value", "");
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				videoSourceComponent->getUSBVideoSourceDefinition()->setDevicePath(newDevicePath);

				// Refresh the video mode list for the newly selected device				
				m_videoModeNameList->rebuildList(true);

				// Refresh all video settings for the new device
				refreshSettings();
			}
		});

	constructor.BindEventCallback(
		"select_video_mode_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto newVideoMode = ev.GetParameter<Rml::String>("value", "");
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				videoSourceComponent->getUSBVideoSourceDefinition()->setVideoMode(newVideoMode);

				// Refresh all video settings for the new video mode
				refreshSettings();
			}
		});

	return true;
}

bool RmlModel_USBVideoSourceComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		auto usbVideoSourceComponent = std::static_pointer_cast<USBVideoSourceComponent>(component);

		// Update all video setting bindings with the new component
		for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
		{
			RmlDataBinding_VideoSourceSettingPtr videoSettingBinding = m_videoSourceSettings[settingIndex];

			videoSettingBinding->setVideoSourceComponent(usbVideoSourceComponent);
		}

		m_usbDevicePathList->setOwnerConfig(getVideoSourceSystemConfig());
		m_usbDevicePathList->rebuildList(true);

		m_videoModeNameList->setOwnerConfig(component ? component->getDefinition() : CommonConfigPtr());
		m_videoModeNameList->rebuildList(true);
		
		return true;
	}

	return false;
}

VideoSourceSystemPtr RmlModel_USBVideoSourceComponent::getVideoSourceSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<VideoSourceSystem>();
	}

	return nullptr;
}

VideoSourceSystemConfigPtr RmlModel_USBVideoSourceComponent::getVideoSourceSystemConfig() const
{
	auto videoSourceSystem = getVideoSourceSystem();
	if (videoSourceSystem)
	{
		return videoSourceSystem->getVideoSourceSystemConfig();
	}

	return nullptr;
}

USBVideoSourceSystemPtr RmlModel_USBVideoSourceComponent::getUSBVideoSourceSystem() const
{
	auto videoSourceSystem = getVideoSourceSystem();
	if (videoSourceSystem)
	{
		return videoSourceSystem->getUSBVideoSourceSystem();
	}

	return nullptr;
}

USBVideoSourceComponentPtr RmlModel_USBVideoSourceComponent::getUSBVideoSourceComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<USBVideoSourceComponent>(component);
	}

	return nullptr;
}

void RmlModel_USBVideoSourceComponent::refreshSettings()
{
	for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
	{
		RmlDataBinding_VideoSourceSettingPtr videoSettingBinding = m_videoSourceSettings[settingIndex];

		videoSettingBinding->refreshDataBinding();
	}
}