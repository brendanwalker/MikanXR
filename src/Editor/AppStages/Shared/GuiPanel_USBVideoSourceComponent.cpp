#include "AppStage.h"
#include "Shared/GuiPanel_USBVideoSourceComponent.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"

#include "imgui.h"

bool GuiPanel_USBVideoSourceComponent::init(AppStage* ownerAppStage)
{
	m_usbVideoSourceSystem = ownerAppStage->getObjectSystemOfType<USBVideoSourceSystem>();
	return initTypedPropertyInterface<USBVideoSourceComponent>(ownerAppStage);
}

bool GuiPanel_USBVideoSourceComponent::setComponent(MikanComponentPtr component)
{
	if (GuiPanel_MikanComponent::setComponent(component))
	{
		m_usbDevicePaths.clear();
		m_videoResolutions.clear();
		m_videoFrameRates.clear();
		m_videoFormats.clear();

		auto usbSystem = getUSBVideoSourceSystem();
		if (usbSystem)
		{
			usbSystem->getConnectedUSBVideoSourcePaths(m_usbDevicePaths);
		}

		auto usbComp = getUSBVideoSourceComponent();
		if (usbComp)
		{
			usbComp->getVideoResolutionNames(m_videoResolutions);
			usbComp->getVideoFrameRateNames(m_videoFrameRates);
			usbComp->getVideoFormatNames(m_videoFormats);
		}

		return true;
	}

	return false;
}

void GuiPanel_USBVideoSourceComponent::render(float deltaSeconds)
{
	GuiPanel_MikanComponent::render(deltaSeconds);

	auto usbComp = getUSBVideoSourceComponent();
	if (!usbComp)
		return;

	ImGui::Separator();

	// USB device path dropdown
	{
		const std::string& currentDevicePath = usbComp->getUSBVideoSourceDefinition()->getDevicePath();
		const char* previewLabel = currentDevicePath.empty() ? "None" : currentDevicePath.c_str();

		// Refresh device paths from system
		auto usbSystem = getUSBVideoSourceSystem();
		std::vector<std::string> devicePaths;
		if (usbSystem)
		{
			usbSystem->getConnectedUSBVideoSourcePaths(devicePaths);
		}

		if (ImGui::BeginCombo("USB Device", previewLabel))
		{
			for (const std::string& devicePath : devicePaths)
			{
				const bool bSelected = (devicePath == currentDevicePath);
				if (ImGui::Selectable(devicePath.c_str(), bSelected))
				{
					addUpdateCallback([usbComp, devicePath]() {
						usbComp->getUSBVideoSourceDefinition()->setDevicePath(devicePath);
					});
				}
			}
			ImGui::EndCombo();
		}
	}

	// Video resolution dropdown
	{
		std::string currentResolution;
		usbComp->getVideoModeResolutionName(currentResolution);

		std::vector<std::string> resolutions;
		usbComp->getVideoResolutionNames(resolutions);

		if (ImGui::BeginCombo("Resolution", currentResolution.c_str()))
		{
			for (const std::string& resolution : resolutions)
			{
				const bool bSelected = (resolution == currentResolution);
				if (ImGui::Selectable(resolution.c_str(), bSelected))
				{
					addUpdateCallback([usbComp, resolution]() {
						std::string frameRate, format;
						usbComp->getVideoModeFrameRateName(frameRate);
						usbComp->getVideoModeFormatName(format);
						usbComp->setVideoModeToBestMatch(resolution, frameRate, format);
					});
				}
			}
			ImGui::EndCombo();
		}
	}

	// Video frame rate dropdown
	{
		std::string currentFrameRate;
		usbComp->getVideoModeFrameRateName(currentFrameRate);

		std::vector<std::string> frameRates;
		usbComp->getVideoFrameRateNames(frameRates);

		if (ImGui::BeginCombo("Frame Rate", currentFrameRate.c_str()))
		{
			for (const std::string& frameRate : frameRates)
			{
				const bool bSelected = (frameRate == currentFrameRate);
				if (ImGui::Selectable(frameRate.c_str(), bSelected))
				{
					addUpdateCallback([usbComp, frameRate]() {
						std::string resolution, format;
						usbComp->getVideoModeResolutionName(resolution);
						usbComp->getVideoModeFormatName(format);
						usbComp->setVideoModeToBestMatch(resolution, frameRate, format);
					});
				}
			}
			ImGui::EndCombo();
		}
	}

	// Video format dropdown
	{
		std::string currentFormat;
		usbComp->getVideoModeFormatName(currentFormat);

		std::vector<std::string> formats;
		usbComp->getVideoFormatNames(formats);

		if (ImGui::BeginCombo("Format", currentFormat.c_str()))
		{
			for (const std::string& format : formats)
			{
				const bool bSelected = (format == currentFormat);
				if (ImGui::Selectable(format.c_str(), bSelected))
				{
					addUpdateCallback([usbComp, format]() {
						std::string resolution, frameRate;
						usbComp->getVideoModeResolutionName(resolution);
						usbComp->getVideoModeFrameRateName(frameRate);
						usbComp->setVideoModeToBestMatch(resolution, frameRate, format);
					});
				}
			}
			ImGui::EndCombo();
		}
	}
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
