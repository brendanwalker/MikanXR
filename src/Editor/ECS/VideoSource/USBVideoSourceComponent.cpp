#include "USBVideoSourceComponent.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- USBVideoSourceDefinition -----
const std::string USBVideoSourceDefinition::k_devicePathPropertyId = "devicePath";
const std::string USBVideoSourceDefinition::k_videoModePropertyId = "videoMode";
const std::string USBVideoSourceDefinition::k_brightnessPropertyId = "brightness";

USBVideoSourceDefinition::USBVideoSourceDefinition()
	: VideoSourceDefinition()
	, m_devicePath("")
	, m_videoMode("")
	, m_brightness(-1.f)
{}

USBVideoSourceDefinition::USBVideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const MikanUSBVideoSourceInfo& videoSourceInfo)
	: VideoSourceDefinition(
		videoSourceId, 
		videoSourceInfo.usb_source_name.getValue(),
		videoSourceInfo.intrinsics)
	, m_devicePath(videoSourceInfo.device_path.getValue())
	, m_videoMode(videoSourceInfo.video_mode.getValue())
	, m_brightness(-1.f)
{}

configuru::Config USBVideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = VideoSourceDefinition::writeToJSON();

	pt["device_path"] = m_devicePath;
	pt["video_mode"] = m_videoMode;
	pt["brightness"] = m_brightness;

	return pt;
}

void USBVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);
	m_devicePath = pt.get_or<std::string>("device_path", m_devicePath);
	m_videoMode = pt.get_or<std::string>("video_mode", m_videoMode);
	m_brightness = pt.get_or<float>("brightness", m_brightness);
}

void USBVideoSourceDefinition::setDevicePath(const std::string& devicePath)
{
	if (devicePath != m_devicePath)
	{
		m_devicePath = devicePath;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_devicePathPropertyId));
	}
}

void USBVideoSourceDefinition::setVideoMode(const std::string& videoMode)
{
	if (videoMode != m_videoMode)
	{
		m_videoMode = videoMode;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_videoModePropertyId));
	}
}

void USBVideoSourceDefinition::setBrightness(const float brightness)
{
	if (brightness != m_brightness)
	{
		m_brightness = brightness;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_brightnessPropertyId));
	}
}

// -- USBVideoSourceComponent -----
USBVideoSourceComponent::USBVideoSourceComponent(MikanObjectWeakPtr owner)
	: VideoSourceComponent(owner)
{}

void USBVideoSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	// Close any open video source that was open
	closeVideoSource();
}

bool USBVideoSourceComponent::openVideoSource()
{
	return false;
}

void USBVideoSourceComponent::closeVideoSource()
{

}

eVideoStreamingStatus USBVideoSourceComponent::startVideoStream()
{
	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus USBVideoSourceComponent::getVideoStreamingStatus() const
{
	return eVideoStreamingStatus::failed;
}

void USBVideoSourceComponent::stopVideoStream()
{
}

bool USBVideoSourceComponent::hasNewVideoFrameAvailable(VideoFrameSection section) const
{
	return false;
}

int64_t USBVideoSourceComponent::readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer)
{
	return 0;
}

void USBVideoSourceComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(USBVideoSourceDefinition::k_devicePathPropertyId);
	outPropertyNames.push_back(USBVideoSourceDefinition::k_videoModePropertyId);
	outPropertyNames.push_back(USBVideoSourceDefinition::k_brightnessPropertyId);
}

bool USBVideoSourceComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (MikanComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == USBVideoSourceDefinition::k_devicePathPropertyId)
	{
		outDescriptor = {USBVideoSourceDefinition::k_devicePathPropertyId, ePropertyDataType::datatype_string, ePropertySemantic::filename};
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_videoModePropertyId)
	{
		outDescriptor = {USBVideoSourceDefinition::k_videoModePropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name};
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_brightnessPropertyId)
	{
		outDescriptor = {USBVideoSourceDefinition::k_brightnessPropertyId, ePropertyDataType::datatype_float, ePropertySemantic::size1d};
		return true;
	}
	return false;
}

bool USBVideoSourceComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == USBVideoSourceDefinition::k_devicePathPropertyId)
	{
		outValue = getUSBVideoSourceDefinition()->getDevicePath();
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_videoModePropertyId)
	{
		outValue = getUSBVideoSourceDefinition()->getVideoMode();
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_brightnessPropertyId)
	{
		outValue = getUSBVideoSourceDefinition()->getBrightness();
		return true;
	}
	return false;
}

bool USBVideoSourceComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (MikanComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == USBVideoSourceDefinition::k_devicePathPropertyId)
	{
		std::string devicePath = inValue.Get<std::string>();
		getUSBVideoSourceDefinition()->setDevicePath(devicePath);
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_videoModePropertyId)
	{
		std::string videoMode = inValue.Get<std::string>();
		getUSBVideoSourceDefinition()->setVideoMode(videoMode);
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_brightnessPropertyId)
	{
		float brightness = inValue.Get<float>();
		getUSBVideoSourceDefinition()->setBrightness(brightness);
		return true;
	}

	return false;
}

const std::string USBVideoSourceComponent::k_calibrateIntrinsicsFunctionId = "calibrate_intrinsics";
const std::string USBVideoSourceComponent::k_testIntrinsicsFunctionId = "test_intrinsics";

void USBVideoSourceComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_calibrateIntrinsicsFunctionId);
	outPropertyNames.push_back(k_testIntrinsicsFunctionId);
}

bool USBVideoSourceComponent::getFunctionDescriptor(
	const std::string& functionName,
	FunctionDescriptor& outDescriptor) const
{
	if (MikanComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == k_calibrateIntrinsicsFunctionId)
	{
		outDescriptor = {k_calibrateIntrinsicsFunctionId, "Calibrate Intrinsics"};
		return true;
	}
	else if (functionName == k_testIntrinsicsFunctionId)
	{
		outDescriptor = {k_testIntrinsicsFunctionId, "Test Intrinsics"};
		return true;
	}
	return false;
}

bool USBVideoSourceComponent::invokeFunction(const std::string& functionName)
{
	if (MikanComponent::invokeFunction(functionName))
		return true;

	if (functionName == k_calibrateIntrinsicsFunctionId)
	{
		calibrateIntrinsics();
		return true;
	}
	else if (functionName == k_testIntrinsicsFunctionId)
	{
		testIntrinsics();
		return true;
	}
	return false;
}

void USBVideoSourceComponent::calibrateIntrinsics()
{
	//TODO
}

void USBVideoSourceComponent::testIntrinsics()
{
	//TODO
}