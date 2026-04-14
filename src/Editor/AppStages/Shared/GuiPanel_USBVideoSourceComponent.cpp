#include "AppStage.h"
#include "Shared/GuiPanel_USBVideoSourceComponent.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"

std::set<std::string> GuiPanel_USBVideoSourceComponent::k_compactGuiPropertiesSet = {
	MikanComponentDefinition::k_componentNamePropertyId,
	USBVideoSourceComponent::k_currentFriendlyNamePropertyId,
	USBVideoSourceDefinition::k_videoResolutionPropertyId,
	USBVideoSourceDefinition::k_videoFrameRatePropertyId,
	USBVideoSourceDefinition::k_videoFormatPropertyId
};

std::set<std::string> GuiPanel_USBVideoSourceComponent::k_compactGuiFunctionsSet = {
	USBVideoSourceComponent::k_showVideoSourceSettingsFunctionId
};

bool GuiPanel_USBVideoSourceComponent::init()
{
	m_usbVideoSourceSystem = getOwnerAppStage()->getObjectSystemOfType<USBVideoSourceSystem>();
	return initTypedPropertyInterface<USBVideoSourceComponent>();
}

void GuiPanel_USBVideoSourceComponent::onConstruct()
{
	// USB device path dropdown
	m_entityAccessor->setPropertyRenderer(
		USBVideoSourceDefinition::k_desiredDevicePathPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto usbComp = getUSBVideoSourceComponent();
			if (!usbComp) return false;

			auto usbSystem = getUSBVideoSourceSystem();
			std::vector<std::string> devicePaths;
			if (usbSystem)
				usbSystem->getConnectedUSBVideoSourcePaths(devicePaths);
			m_devicePathDataSource.setEntries(devicePaths);

			const std::string& currentPath = usbComp->getUSBVideoSourceDefinition()->getDevicePath();
			int selectedIndex = m_devicePathDataSource.getEntryIndexByString(currentPath);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "usbDevicePath", "USB Device",
				&m_devicePathDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const std::string newPath = m_devicePathDataSource.getEntryDisplayString(selectedIndex);
					addDeferredGuiEvent([usbComp, newPath]() {
						usbComp->getUSBVideoSourceDefinition()->setDevicePath(newPath);
					});
				}
			}
			return true;
		});

	// Video resolution dropdown
	m_entityAccessor->setPropertyRenderer(
		USBVideoSourceDefinition::k_videoResolutionPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto usbComp = getUSBVideoSourceComponent();
			if (!usbComp) return false;

			std::vector<std::string> resolutions;
			usbComp->getVideoResolutionNames(resolutions);
			m_resolutionDataSource.setEntries(resolutions);

			std::string currentResolution;
			usbComp->getVideoModeResolutionName(currentResolution);
			int selectedIndex = m_resolutionDataSource.getEntryIndexByString(currentResolution);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "usbVideoResolution", "Resolution",
				&m_resolutionDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const std::string newResolution =
						m_resolutionDataSource.getEntryDisplayString(selectedIndex);
					addDeferredGuiEvent([usbComp, newResolution]() {
						std::string frameRate, format;
						usbComp->getVideoModeFrameRateName(frameRate);
						usbComp->getVideoModeFormatName(format);
						usbComp->setVideoModeToBestMatch(newResolution, frameRate, format);
					});
				}
			}
			return true;
		});

	// Video frame rate dropdown
	m_entityAccessor->setPropertyRenderer(
		USBVideoSourceDefinition::k_videoFrameRatePropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto usbComp = getUSBVideoSourceComponent();
			if (!usbComp) return false;

			std::vector<std::string> frameRates;
			usbComp->getVideoFrameRateNames(frameRates);
			m_frameRateDataSource.setEntries(frameRates);

			std::string currentFrameRate;
			usbComp->getVideoModeFrameRateName(currentFrameRate);
			int selectedIndex = m_frameRateDataSource.getEntryIndexByString(currentFrameRate);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "usbVideoFrameRate", "Frame Rate",
				&m_frameRateDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const std::string newFrameRate =
						m_frameRateDataSource.getEntryDisplayString(selectedIndex);
					addDeferredGuiEvent([usbComp, newFrameRate]() {
						std::string resolution, format;
						usbComp->getVideoModeResolutionName(resolution);
						usbComp->getVideoModeFormatName(format);
						usbComp->setVideoModeToBestMatch(resolution, newFrameRate, format);
					});
				}
			}
			return true;
		});

	// Video format dropdown
	m_entityAccessor->setPropertyRenderer(
		USBVideoSourceDefinition::k_videoFormatPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto usbComp = getUSBVideoSourceComponent();
			if (!usbComp) return false;

			std::vector<std::string> formats;
			usbComp->getVideoFormatNames(formats);
			m_formatDataSource.setEntries(formats);

			std::string currentFormat;
			usbComp->getVideoModeFormatName(currentFormat);
			int selectedIndex = m_formatDataSource.getEntryIndexByString(currentFormat);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "usbVideoFormat", "Format",
				&m_formatDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const std::string newFormat =
						m_formatDataSource.getEntryDisplayString(selectedIndex);
					addDeferredGuiEvent([usbComp, newFormat]() {
						std::string resolution, frameRate;
						usbComp->getVideoModeResolutionName(resolution);
						usbComp->getVideoModeFrameRateName(frameRate);
						usbComp->setVideoModeToBestMatch(resolution, frameRate, newFormat);
					});
				}
			}
			return true;
		});
}

USBVideoSourceSystemPtr GuiPanel_USBVideoSourceComponent::getUSBVideoSourceSystem() const
{
	return m_usbVideoSourceSystem.lock();
}

USBVideoSourceComponentPtr GuiPanel_USBVideoSourceComponent::getUSBVideoSourceComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<USBVideoSourceComponent>(component);
	}
	return nullptr;
}

void GuiPanel_USBVideoSourceComponent::drawCompactGui()
{
	GuiPanel_EntityAccessorPtr entityAccessor= getPropertyInterface();

	entityAccessor->drawPropertiesGui(k_compactGuiPropertiesSet);
	entityAccessor->drawFunctionsGui(k_compactGuiFunctionsSet);
}