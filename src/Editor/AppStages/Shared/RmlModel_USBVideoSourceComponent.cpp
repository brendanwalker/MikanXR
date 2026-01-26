#include "AppStage.h"
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
	, m_videoResolutionList(std::make_shared<RmlDataBinding_VideoResolutionList>())
	, m_videoFrameRateList(std::make_shared<RmlDataBinding_VideoFrameRateList>())
	, m_videoFormatList(std::make_shared<RmlDataBinding_VideoFormatList>())
{
	for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
	{
		m_videoSourceSettings[settingIndex] =
			std::make_shared<RmlDataBinding_VideoSourceSetting>(
				this,
				static_cast<eVideoSettingType>(settingIndex));
	}
}

bool RmlModel_USBVideoSourceComponent::init(AppStage* ownerAppStage)
{
	m_usbVideoSourceSystem = ownerAppStage->getObjectSystemOfType<USBVideoSourceSystem>();

	return initTypedPropertyInterface<USBVideoSourceComponent>(ownerAppStage->getRmlContext());
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

	// Build the list of video resolutions from the currently selected USBVideoSourceComponent
	m_videoResolutionList->init(
		constructor,
		CommonConfigPtr(),
		"video_resolutions",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<std::string>& outVideoResolutionList) {
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				videoSourceComponent->getVideoResolutionNames(outVideoResolutionList);
			}
		},
		[this](const ConfigPropertyChangeSet& changedPropertySet) -> bool {
			return changedPropertySet.hasPropertyName(USBVideoSourceComponent::k_currentDevicePathPropertyId);
		});
	m_videoFrameRateList->init(
		constructor,
		CommonConfigPtr(),
		"video_frame_rates",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<std::string>& outVideoFrameRateList) {
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				videoSourceComponent->getVideoFrameRateNames(outVideoFrameRateList);
			}
		},
		[this](const ConfigPropertyChangeSet& changedPropertySet) -> bool {
			return changedPropertySet.hasPropertyName(USBVideoSourceComponent::k_currentDevicePathPropertyId);
		});
	m_videoFormatList->init(
		constructor,
		CommonConfigPtr(),
		"video_formats",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<std::string>& outVideoFormatList) {
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				videoSourceComponent->getVideoFormatNames(outVideoFormatList);
			}
		},
		[this](const ConfigPropertyChangeSet& changedPropertySet) -> bool {
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
		"select_video_resolution_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto newResolutionMode = ev.GetParameter<Rml::String>("value", "");
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				std::string frameRate;
				std::string format;

				videoSourceComponent->getVideoModeFrameRateName(frameRate);
				videoSourceComponent->getVideoModeFormatName(format);
				videoSourceComponent->setVideoModeToBestMatch(newResolutionMode, frameRate, format);
			}
		});
	constructor.BindEventCallback(
		"select_video_fps_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto newFPS = ev.GetParameter<Rml::String>("value", "");
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				std::string resolution;
				std::string format;

				videoSourceComponent->getVideoModeResolutionName(resolution);
				videoSourceComponent->getVideoModeFormatName(format);
				videoSourceComponent->setVideoModeToBestMatch(resolution, newFPS, format);
			}
		});
	constructor.BindEventCallback(
		"select_video_format_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto newVideoFormat = ev.GetParameter<Rml::String>("value", "");
			auto videoSourceComponent = getUSBVideoSourceComponent();
			if (videoSourceComponent)
			{
				std::string resolution;
				std::string frameRate;

				videoSourceComponent->getVideoModeResolutionName(resolution);
				videoSourceComponent->getVideoModeFrameRateName(frameRate);
				videoSourceComponent->setVideoModeToBestMatch(resolution, frameRate, newVideoFormat);
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

		m_videoResolutionList->setOwnerConfig(component ? component->getDefinition() : CommonConfigPtr());
		m_videoResolutionList->rebuildList(true);

		m_videoFrameRateList->setOwnerConfig(component ? component->getDefinition() : CommonConfigPtr());
		m_videoFrameRateList->rebuildList(true);

		m_videoFormatList->setOwnerConfig(component ? component->getDefinition() : CommonConfigPtr());
		m_videoFormatList->rebuildList(true);

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
	return m_usbVideoSourceSystem.lock();
}

USBVideoSourceSystemDefinitionPtr RmlModel_USBVideoSourceComponent::getUSBVideoSourceSystemConfig() const
{
	auto videoSourceSystem = getUSBVideoSourceSystem();
	if (videoSourceSystem)
	{
		return videoSourceSystem->getTypedDefinition();
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