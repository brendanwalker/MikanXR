#include "CameraMath.h"
#include "MathUtility.h"
#include "MikanServer.h"
#include "OpenCVVideoFrameBuffer.h"
#include "ThreadUtils.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceRequestHandler.h"
#include "VideoSourceSystem.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include <assert.h>

// -- USBVideoSourceDefinition -----
const std::string USBVideoSourceDefinition::k_devicePathPropertyId = "devicePath";
const std::string USBVideoSourceDefinition::k_videoModePropertyId = "videoMode";
const std::string USBVideoSourceDefinition::k_cameraSettingsPropertyId = "cameraSettings";

USBVideoSourceDefinition::USBVideoSourceDefinition()
	: VideoSourceDefinition()
	, m_devicePath("")
	, m_videoMode("")
{	
}

USBVideoSourceDefinition::USBVideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const MikanUSBVideoSourceInfo& videoSourceInfo)
	: VideoSourceDefinition(
		videoSourceId, 
		videoSourceInfo.usb_source_name.getValue(),
		videoSourceInfo.intrinsics)
	, m_devicePath(videoSourceInfo.device_path.getValue())
	, m_videoMode(videoSourceInfo.video_mode.getValue())
{
}

configuru::Config USBVideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = VideoSourceDefinition::writeToJSON();

	pt["device_path"] = m_devicePath;
	pt["video_mode"] = m_videoMode;
	writeStdArrayMap<float, (int)eVideoSettingType::COUNT>(pt, "cameraSettings", m_videoSettingsMap);

	return pt;
}

void USBVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);
	m_devicePath = pt.get_or<std::string>("device_path", m_devicePath);
	m_videoMode = pt.get_or<std::string>("video_mode", m_videoMode);
	readStdArrayMap<float, (int)eVideoSettingType::COUNT>(pt, "cameraSettings", m_videoSettingsMap);
}

void USBVideoSourceDefinition::setDevicePath(const std::string& devicePath)
{
	if (devicePath != m_devicePath)
	{
		m_devicePath = devicePath;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_devicePathPropertyId));
	}
}

void USBVideoSourceDefinition::setVideoMode(const std::string& videoMode)
{
	if (videoMode != m_videoMode)
	{
		m_videoMode = videoMode;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_videoModePropertyId));
	}
}

bool USBVideoSourceDefinition::getVideoSettingValue(
	const std::string& modeName,
	eVideoSettingType settingType,
	float& outValue) const
{
	if (m_videoSettingsMap.find(modeName) != m_videoSettingsMap.end())
	{
		const auto& settings = m_videoSettingsMap.at(modeName);
		
		outValue= settings[(int)settingType];
		return true; // Found the setting value
	}

	return false; // Default value if not found
}

void USBVideoSourceDefinition::setCameraSettingValue(
	const std::string& modeName,
	eVideoSettingType settingType,
	float value,
	bool bBroadcastPropertyChange)
{
	bool bSettingsChanged = false;

	auto it = m_videoSettingsMap.find(modeName);
	if (it == m_videoSettingsMap.end())
	{
		 auto settings = std::array<float, (int)eVideoSettingType::COUNT>();

		 settings[(int)settingType] = value;
		 m_videoSettingsMap[modeName] = settings;
		 bSettingsChanged = true;
	}
	else
	{
		auto& settings = it->second;

		if (settings[(int)settingType] != value)
		{
			settings[(int)settingType] = value;
			bSettingsChanged = true;
		}
	}

	if (bSettingsChanged && bBroadcastPropertyChange)
	{
		notifyCameraSettingsChanged();
	}
}

void USBVideoSourceDefinition::notifyCameraSettingsChanged()
{
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_cameraSettingsPropertyId));
}

// -- USBVideoSourceComponent -----
USBVideoSourceComponent::USBVideoSourceComponent(MikanObjectWeakPtr owner)
	: VideoSourceComponent(owner)
	, m_usbVideoDevice(nullptr)
{
}

void USBVideoSourceComponent::dispose()
{
	// Close the video source if it is open
	closeVideoSource();

	// Call the base class dispose method
	MikanComponent::dispose();
}

void USBVideoSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	// Close any open video source that was open
	closeVideoSource();
}

std::string USBVideoSourceComponent::getDevicePath() const
{
	if (m_usbVideoDevice != nullptr)
	{
		return m_usbVideoDevice->getDevicePath();
	}

	return "";
}

std::string USBVideoSourceComponent::getDeviceAPI() const
{
	return "USBVideoSource";
}

bool USBVideoSourceComponent::openVideoSource()
{
	// If the video source is already open, do nothing
	if (m_usbVideoDevice != nullptr)
		return true;

	// If the device path is empty, return false
	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	const std::string& devicePath = definition->getDevicePath();
	if (devicePath.empty())
		return false;

	// Find the USB video device by its path
	VideoSourceSystemPtr videoSourceSystem= VideoSourceSystem::getSystem();
	USBVideoSourceSystemPtr usbVideoSourceSystem= videoSourceSystem->getUSBVideoSourceSystem();
	IUsbVideoDeviceManagerPtr usbVideoDeviceManager = usbVideoSourceSystem->getUSBVideoDeviceManager();

	m_usbVideoDevice= usbVideoDeviceManager->getDeviceByPath(devicePath.c_str());
	if (m_usbVideoDevice == nullptr)
		return false;

	// Attempt to open the USB video device
	if (!m_usbVideoDevice->open())
		return false;

	// Apply the desired video mode from the definition to the USB video device
	if (!updateVideoMode())
		return false;

	// Apply camera settings from the definition to the USB video device
	updateCameraSettings();

	// Listen for events from the USB video device
	m_usbVideoDevice->addListener(this);

	// Apply the side effect of video mode changes 
	notifyVideoModePropertiesChanged(m_usbVideoDevice);
	if (OnFrameSizeChanged)
	{
		OnFrameSizeChanged(getSelfPtr<VideoSourceComponent>());
	}

	// Let any connected clients know that the video source closed
	MikanServer::getInstance()->getVideoSourceRequestHandler()->publishVideoSourceOpenedEvent();
	if (OnOpened)
	{
		OnOpened(getSelfPtr<VideoSourceComponent>());
	}

	return true;
}

bool USBVideoSourceComponent::updateVideoMode()
{
	if (m_usbVideoDevice == nullptr)
		return false;

	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	const std::string desiredVideoMode = definition->getVideoMode();
	const std::string currentVideoMode = m_usbVideoDevice->getVideoModeName();
	if (desiredVideoMode != currentVideoMode)
	{
		// If no video mode is set, then set the first available video mode
		if (currentVideoMode.empty() && desiredVideoMode.empty())
		{
			// Set the first available video mode
			UsbVideoModeProperties modeProperties;
			if (m_usbVideoDevice->getVideoModeProperties(0, modeProperties) &&
				m_usbVideoDevice->setVideoModeByIndex(0))
			{
				// Update the definition with the video mode name
				definition->setVideoMode(modeProperties.name);
			}
			else
			{
				// If no video modes are available, return false
				return false;
			}
		}
		// If we have a valid desired video mode, apply it
		else if (!desiredVideoMode.empty())
		{
			if (!m_usbVideoDevice->setVideoModeByName(desiredVideoMode.c_str()))
			{
				// If the desired video mode could not be set, return false
				definition->setVideoMode("");
				return false;
			}
		}
		// If we have a valid current video mode, update the definition
		else
		{
			definition->setVideoMode(currentVideoMode);
		}
	}

	return true;
}

void USBVideoSourceComponent::updateCameraSettings()
{
	if (m_usbVideoDevice == nullptr)
		return;

	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	const std::string videoMode= m_usbVideoDevice->getVideoModeName();
	bool bModifiedCameraSettings = false;

	// Apply video property settings stored in config onto the camera
	// Or store the current camera settings if not set
	for (int prop_index = 0; prop_index < (int)eVideoSettingType::COUNT; ++prop_index)
	{
		const eVideoSettingType prop_type = (eVideoSettingType)prop_index;
		
		if (VideoSettingConstraint constraint;
			m_usbVideoDevice->getVideoSettingConstraint(prop_type, constraint))
		{
			float desiredFloatValue = 0.f;
			if (definition->getVideoSettingValue(
				videoMode,
				prop_type,
				desiredFloatValue))
			{
				const int desiredIntValue= 
					remap_float_to_int(
						0.f, 1.f,
						constraint.min_value, constraint.max_value,
						desiredFloatValue);

				// Set the camera setting on the USB video device
				m_usbVideoDevice->setVideoSetting(prop_type, desiredIntValue);
			}
			else
			{
				const int currentIntValue= m_usbVideoDevice->getVideoSetting(prop_type);
				const float currentFloatValue= 
					remap_int_to_float(
						constraint.min_value, constraint.max_value,
						0.f, 1.f,
						currentIntValue);

				definition->setCameraSettingValue(
					videoMode, 
					prop_type, 
					currentFloatValue, 
					false); // Do not broadcast dirty state here
				bModifiedCameraSettings = true;
			}
		}
	}

	if (bModifiedCameraSettings)
	{
		// Mark the camera settings as dirty if any settings were modified
		definition->notifyCameraSettingsChanged();
	}
}

void USBVideoSourceComponent::closeVideoSource()
{
	if (m_usbVideoDevice != nullptr)
	{
		// Stop the video stream if it is running
		stopVideoStream();

		// Close the USB video device
		m_usbVideoDevice->close();

		// Remove the listener for the USB video device
		m_usbVideoDevice->removeListener(this);

		// Clear the USB video device pointer
		m_usbVideoDevice = nullptr;
	}

	// Release any OpenCV buffer state
	releaseOpencvBufferState();

	// Let any connected clients know that the video source closed
	MikanServer::getInstance()->getVideoSourceRequestHandler()->publishVideoSourceClosedEvent();
	if (OnClosed)
	{
		OnClosed(getSelfPtr<VideoSourceComponent>());
	}
}

eVideoStreamingStatus USBVideoSourceComponent::startVideoStream()
{
	if (m_usbVideoDevice != nullptr)
	{
		eVideoStreamingStatus status= m_usbVideoDevice->startVideoStream();
		if (status == eVideoStreamingStatus::started && OnStarted)
		{
			OnStarted(getSelfPtr<VideoSourceComponent>());
		}

		return status;
	}
	
	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus USBVideoSourceComponent::getVideoStreamingStatus() const
{
	if (m_usbVideoDevice != nullptr)
	{
		return m_usbVideoDevice->getVideoStreamingStatus();
	}

	return eVideoStreamingStatus::failed;
}

void USBVideoSourceComponent::stopVideoStream()
{
	if (m_usbVideoDevice != nullptr)
	{
		m_usbVideoDevice->stopVideoStream();

		if (OnStopped)
		{
			OnStopped(getSelfPtr<VideoSourceComponent>());
		}
	}
}

bool USBVideoSourceComponent::isVideoSettingSupported(const eVideoSettingType property_type) const
{
	if (m_usbVideoDevice != nullptr)
	{
		return m_usbVideoDevice->isVideoSettingSupported(property_type);
	}

	return false;
}

bool USBVideoSourceComponent::getVideoSettingConstraint(const eVideoSettingType property_type, VideoSettingConstraint& outConstraint) const
{
	if (m_usbVideoDevice != nullptr)
	{
		return m_usbVideoDevice->getVideoSettingConstraint(property_type, outConstraint);
	}

	return false;
}

void USBVideoSourceComponent::setVideoSetting(const eVideoSettingType property_type, int desired_value)
{
	if (m_usbVideoDevice != nullptr)
	{
		m_usbVideoDevice->setVideoSetting(property_type, desired_value);
	}
}

int USBVideoSourceComponent::getVideoSetting(const eVideoSettingType property_type) const
{
	if (m_usbVideoDevice != nullptr)
	{
		return m_usbVideoDevice->getVideoSetting(property_type);
	}

	return -1;
}

bool USBVideoSourceComponent::getFrameRate(float& outFrameRate) const
{
	UsbVideoModeProperties modeProperties;
	if (getVideoModeProperties(getVideoModeIndex(), modeProperties))
	{
		outFrameRate =
			(float)modeProperties.frame_rate_numerator / 
			(float)modeProperties.frame_rate_demonenator;
		return true;
	}

	return false;
}

bool USBVideoSourceComponent::getVideoModeName(std::string& outVideoModeName) const
{
	if (m_usbVideoDevice != nullptr)
	{
		outVideoModeName= m_usbVideoDevice->getVideoModeName();
		return true;
	}

	return false;
}

bool USBVideoSourceComponent::getVideoPixelDimensions(int& outPixelWidth, int& outPixelHeight) const
{
	UsbVideoModeProperties modeProperties;
	if (getVideoModeProperties(getVideoModeIndex(), modeProperties))
	{
		outPixelWidth = modeProperties.width;
		outPixelHeight = modeProperties.height;
		return true;
	}

	return false;
}

size_t USBVideoSourceComponent::getAvailableVideoModesCount() const
{
	if (m_usbVideoDevice != nullptr)
	{
		return m_usbVideoDevice->getAvailableVideoModesCount();
	}

	return 0;
}

bool USBVideoSourceComponent::getVideoModeProperties(
	size_t index,
	UsbVideoModeProperties& outProperties) const
{
	if (m_usbVideoDevice != nullptr)
	{
		return m_usbVideoDevice->getVideoModeProperties(index, outProperties);
	}

	return false;
}

int USBVideoSourceComponent::getVideoModeIndex() const
{
	if (m_usbVideoDevice != nullptr)
	{
		return m_usbVideoDevice->getVideoModeIndex();
	}

	return -1;
}

bool USBVideoSourceComponent::setVideoModeByName(const std::string& videoModeName)
{
	if (m_usbVideoDevice != nullptr &&
		m_usbVideoDevice->setVideoModeByName(videoModeName.c_str()))
	{
		getUSBVideoSourceDefinition()->setVideoMode(videoModeName);
		return true;
	}

	return false;
}

bool USBVideoSourceComponent::setVideoModeByIndex(size_t index)
{
	if (m_usbVideoDevice != nullptr &&
		m_usbVideoDevice->setVideoModeByIndex(index))
	{
		getUSBVideoSourceDefinition()->setVideoMode(m_usbVideoDevice->getVideoModeName());
		return true;
	}

	return false;
}

bool USBVideoSourceComponent::getVideoModeNames(std::vector<std::string>& outVideoModeNames) const
{
	if (m_usbVideoDevice != nullptr)
	{
		size_t modeCount = m_usbVideoDevice->getAvailableVideoModesCount();
		outVideoModeNames.clear();
		outVideoModeNames.reserve(modeCount);
		for (size_t i = 0; i < modeCount; ++i)
		{
			UsbVideoModeProperties modeProperties;
			if (m_usbVideoDevice->getVideoModeProperties(i, modeProperties))
			{
				outVideoModeNames.push_back(modeProperties.name);
			}
		}

		return true;
	}

	return false;
}

void USBVideoSourceComponent::notifyVideoDeviceDisconnected(const IUsbVideoDevice* device)
{
	if (device == m_usbVideoDevice)
	{
		// The video source is now already invalidated, so we can clear the pointer
		// but we still want to clean up the video source state
		m_usbVideoDevice = nullptr;
		closeVideoSource();
	}
}

void USBVideoSourceComponent::notifyVideoModePropertiesChanged(const class IUsbVideoDevice* device)
{
	// Make sure the device is the one we are currently using
	assert(m_usbVideoDevice == device);

	// At the moment, this function should only be called from video sources that
	// update their video frame size on the main thread.
	// If this changes, we will need to refactor this function to be thread safe.
	assert(ThreadUtils::isRunningInMainThread());

	// Allocate the open cv buffers used for tracking filtering
	reallocateOpencvBufferState();

	// Recompute the projection matrix
	recomputeCameraProjectionMatrix();

	// Let any listeners know that the video frame sized changed
	MikanServer::getInstance()->getVideoSourceRequestHandler()->publishVideoSourceModeChangedEvent();
}

void USBVideoSourceComponent::notifyVideoFrameReceived(const UsbVideoFrameBuffer& bufferInfo)
{
	assert(m_usbVideoDevice != nullptr);

	int videoModeIndex = m_usbVideoDevice->getVideoModeIndex();
	assert(videoModeIndex >= 0);

	UsbVideoModeProperties videoModeProperties;
	bool bVaildVideoMode= m_usbVideoDevice->getVideoModeProperties(videoModeIndex, videoModeProperties);
	assert(bVaildVideoMode);

	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	const MikanVideoSourceIntrinsics& intrinsics = definition->getCameraIntrinsics();

	const bool is_frame_flipped = definition->getIsFrameMirrored();
	const bool is_buffer_flipped = definition->getIsBufferMirrored();

	// Fetch the latest video buffer frame from the device
	if (intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
	{
		const auto& stereoIntrinsics = intrinsics.getStereoIntrinsics();
		const int section_width = (int)stereoIntrinsics.pixel_width;
		const int section_height = (int)stereoIntrinsics.pixel_height;

		cv::Rect left_bounds = cv::Rect(0, 0, section_width, section_height);
		cv::Rect right_bounds = cv::Rect(section_width, 0, section_width, section_height);

		// Cache the left raw video frame
		if (m_opencv_buffer_state[(int)VideoFrameSection::Left] != nullptr)
		{
			m_opencv_buffer_state[(int)VideoFrameSection::Left]->writeStereoVideoFrameSection(
				bufferInfo.data,
				is_buffer_flipped ? right_bounds : left_bounds,
				is_frame_flipped);
		}

		// Cache the right raw video frame
		if (m_opencv_buffer_state[(int)VideoFrameSection::Right] != nullptr)
		{
			m_opencv_buffer_state[(int)VideoFrameSection::Right]->writeStereoVideoFrameSection(
				bufferInfo.data,
				is_buffer_flipped ? left_bounds : right_bounds,
				is_frame_flipped);
		}
	}
	else
	{
		// Cache the raw video frame
		if (m_opencv_buffer_state[(int)VideoFrameSection::Primary] != nullptr)
		{
			m_opencv_buffer_state[(int)VideoFrameSection::Primary]->writeVideoFrame(
				bufferInfo.data, is_frame_flipped);
		}
	}
}

// -- IRmlPropertyInterface ----
void USBVideoSourceComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	VideoSourceComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			USBVideoSourceDefinition::k_devicePathPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			USBVideoSourceDefinition::k_videoModePropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			USBVideoSourceDefinition::k_cameraSettingsPropertyId));
}

bool USBVideoSourceComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

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

	// TODO: k_cameraSettingsPropertyId;

	return VideoSourceComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool USBVideoSourceComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == USBVideoSourceDefinition::k_devicePathPropertyId)
	{
		std::string devicePath = inValue.Get<std::string>();
		getUSBVideoSourceDefinition()->setDevicePath(devicePath);
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_videoModePropertyId)
	{
		std::string videoMode = inValue.Get<std::string>();
		setVideoModeByName(videoMode);
		return true;
	}

	return VideoSourceComponent::setPropertyValueFromRml(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
void USBVideoSourceComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	VideoSourceComponent::getRmlFunctionDescriptors(outDescriptors);
}

bool USBVideoSourceComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	return VideoSourceComponent::invokeFunctionFromRml(functionDesc);
}