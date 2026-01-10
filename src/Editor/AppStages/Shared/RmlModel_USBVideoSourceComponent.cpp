#include "RmlModel_USBVideoSourceComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "NetworkVideoSourceComponent.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceSettings/RmlDataBinding_VideoSourceSetting.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

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
				this,
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
		},
		[this](const ConfigPropertyChangeSet& changedPropertySet) -> bool {
			// We can not rebuild the video mode list if the current device path just updated changed
			return changedPropertySet.hasPropertyName(USBVideoSourceComponent::k_currentDevicePathPropertyId);
		});

	constructor.BindEventCallback(
		"select_video_device_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto newDevicePath = ev.GetParameter<Rml::String>("value", "");
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				videoSourceComponent->getUSBVideoSourceDefinition()->setDevicePath(newDevicePath);
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
			}
		});

	return true;
}

bool RmlModel_USBVideoSourceComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		auto usbVideoSourceComponent = std::static_pointer_cast<USBVideoSourceComponent>(component);

		m_usbDevicePathList->setOwnerConfig(getUSBVideoSourceSystemConfig());
		m_usbDevicePathList->rebuildList(true);

		m_videoModeNameList->setOwnerConfig(component ? component->getDefinition() : CommonConfigPtr());
		m_videoModeNameList->rebuildList(true);

		// Update all video setting bindings with the new component
		for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
		{
			RmlDataBinding_VideoSourceSettingPtr videoSettingBinding = m_videoSourceSettings[settingIndex];

			videoSettingBinding->refreshDataBinding();
		}
		
		return true;
	}

	return false;
}

void RmlModel_USBVideoSourceComponent::onComponentPropertyChanged(
	IEntityAccessorPtr accessorPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(USBVideoSourceDefinition::k_videoSettingsPropertyId))
	{
		for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
		{
			m_videoSourceSettings[settingIndex]->refreshDataBinding();
		}
	}
	else
	{
		RmlModel_MikanComponent::onComponentPropertyChanged(accessorPtr, changedPropertySet);
	}
}

USBVideoSourceSystemPtr RmlModel_USBVideoSourceComponent::getUSBVideoSourceSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<USBVideoSourceSystem>();
	}

	return nullptr;
}

USBVideoSourceSystemConfigPtr RmlModel_USBVideoSourceComponent::getUSBVideoSourceSystemConfig() const
{
	auto videoSourceSystem = getUSBVideoSourceSystem();
	if (videoSourceSystem)
	{
		return videoSourceSystem->getUSBVideoSourceSystemConfig();
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