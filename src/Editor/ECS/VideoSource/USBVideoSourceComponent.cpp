#include "CameraMath.h"
#include "MathUtility.h"
#include "MikanServer.h"
#include "MikanObject.h"
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
const std::string USBVideoSourceDefinition::k_devicePathPropertyId = "device_path";
const std::string USBVideoSourceDefinition::k_videoModePropertyId = "video_mode";
const std::string USBVideoSourceDefinition::k_cameraSettingsPropertyId = "camera_settings";

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
		"VideoSource_"+std::to_string(videoSourceId),
		videoSourceInfo.intrinsics)
	, m_devicePath(videoSourceInfo.device_path.getValue())
	, m_videoMode(videoSourceInfo.video_mode.getValue())
{
}

configuru::Config USBVideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = VideoSourceDefinition::writeToJSON();

	pt[k_devicePathPropertyId] = m_devicePath;
	pt[k_videoModePropertyId] = m_videoMode;
	writeStdArrayMap<float, (int)eVideoSettingType::COUNT>(pt, k_cameraSettingsPropertyId, m_videoSettingsMap);

	return pt;
}

void USBVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);
	m_devicePath = pt.get_or<std::string>(k_devicePathPropertyId, m_devicePath);
	m_videoMode = pt.get_or<std::string>(k_videoModePropertyId, m_videoMode);
	readStdArrayMap<float, (int)eVideoSettingType::COUNT>(pt, k_cameraSettingsPropertyId, m_videoSettingsMap);
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

bool USBVideoSourceDefinition::getVideoSettingsForMode(
	const std::string& modeName,
	USBVideoSettingsArray& outSettings) const
{
	auto it = m_videoSettingsMap.find(modeName);
	if (it != m_videoSettingsMap.end())
	{
		outSettings = it->second;
		return true;
	}

	return false;
}

void USBVideoSourceDefinition::setCameraSettingsForMode(
	const std::string& modeName,
	const USBVideoSettingsArray& settings)
{
	m_videoSettingsMap[modeName] = settings;
}

bool USBVideoSourceDefinition::hasVideoSettingsForMode(const std::string& videoModeName) const
{
	if (!videoModeName.empty())
	{
		return m_videoSettingsMap.find(videoModeName) != m_videoSettingsMap.end();
	}

	return false;
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
	m_currentVideoSettings = {};
	m_currentVideoConstraints = {};
}

void USBVideoSourceComponent::init()
{
	MikanComponent::init();

	// Attempt to open the video source
	openVideoSource();
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

std::string USBVideoSourceComponent::getDeviceFriendlyName() const
{
	if (m_usbVideoDevice != nullptr)
	{
		return m_usbVideoDevice->getFriendlyName();
	}

	return "";
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
	auto usbVideoSourceSystem = std::static_pointer_cast<USBVideoSourceSystem>(getOwnerObject()->getOwnerSystem());
	IUsbVideoDeviceManagerPtr usbVideoDeviceManager = usbVideoSourceSystem->getUSBVideoDeviceManager();

	m_usbVideoDevice= usbVideoDeviceManager->getDeviceByPath(devicePath.c_str());
	if (m_usbVideoDevice == nullptr)
		return false;

	// Apply the desired video mode from the definition to the USB video device
	if (!updateVideoMode())
		return false;

	// Attempt to open the USB video device
	if (!m_usbVideoDevice->open())
		return false;

	// Listen for events from the USB video device
	m_usbVideoDevice->addListener(this);

	// Update settings (brightness, exposure, etc) for the current video mode
	handleVideoModeUpdated();

	// Let any connected clients know that the video source closed
	MikanServer::getInstance()->getVideoSourceRequestHandler()->publishVideoSourceOpenedEvent();
	if (OnOpened)
	{
		OnOpened(getSelfPtr<VideoSourceComponent>());
	}

	return true;
}

void USBVideoSourceComponent::USBVideoSourceComponent::onDefinitionMarkedDirty(
	CommonConfigPtr configPtr, 
	const ConfigPropertyChangeSet& changedPropertySet)
{
	// If the device path changed, reopen the video source
	if (changedPropertySet.hasPropertyName(USBVideoSourceDefinition::k_devicePathPropertyId))
	{
		closeVideoSource();
		openVideoSource();
	}
	// If the video mode changed, update the video mode
	else if (changedPropertySet.hasPropertyName(USBVideoSourceDefinition::k_videoModePropertyId))
	{
		if (updateVideoMode())
		{
			handleVideoModeUpdated();
		}
	}
	// If camera settings changed, update the camera settings
	else if (changedPropertySet.hasPropertyName(USBVideoSourceDefinition::k_cameraSettingsPropertyId))
	{
		handleVideoModeSettingUpdated();
	}
	else
	{
		VideoSourceComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);
	}
}

bool USBVideoSourceComponent::updateVideoMode()
{
	if (m_usbVideoDevice == nullptr)
		return false;

	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	const std::string desiredVideoMode = definition->getVideoMode();
	const char* szCurrentVideoMode = m_usbVideoDevice->getVideoModeName();
	const std::string currentVideoMode = szCurrentVideoMode ? szCurrentVideoMode : "";
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

bool USBVideoSourceComponent::handleVideoModeUpdated()
{
	if (m_usbVideoDevice == nullptr)
		return false;

	const char* szVideoModeName = m_usbVideoDevice->getVideoModeName();
	if (szVideoModeName == nullptr)
		return false;

	// Cache the video setting constraints for the new video mode first
	// (needed by restoreVideoSettingsToCurrentMode / backupVideoSettingsFromCurrentMode)
	for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
	{
		const eVideoSettingType settingType = (eVideoSettingType)settingIndex;

		m_usbVideoDevice->getVideoSettingConstraint(settingType, m_currentVideoConstraints[settingIndex]);
	}

	// See if we have saved camera settings for the current video mode
	bool bCameraPropertiesChanged = false;
	const std::string videoModeName = szVideoModeName;
	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	if (definition->hasVideoSettingsForMode(videoModeName))
	{
		// Apply camera settings from the definition to the USB video device
		restoreVideoSettingsToCurrentMode();
	}
	else
	{
		// Backup current camera settings from the USB video device to the definition
		saveVideoSettingDefaultsFromCurrentMode();
	}

	// Apply the side effect of video mode changes 
	notifyVideoModePropertiesChanged(m_usbVideoDevice);
	if (OnFrameSizeChanged)
	{
		OnFrameSizeChanged(getSelfPtr<VideoSourceComponent>());
	}

	return true;
}

void USBVideoSourceComponent::handleVideoModeSettingUpdated()
{
	if (m_usbVideoDevice == nullptr)
		return;

	const char* szVideoModeName = m_usbVideoDevice->getVideoModeName();
	if (szVideoModeName == nullptr)
		return;

	std::string videoModeName = szVideoModeName;
	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();	
	if (USBVideoSettingsArray settings;
		definition->getVideoSettingsForMode(videoModeName, settings))
	{
		for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
		{
			setVideoSettingAsFloatFraction((eVideoSettingType)settingIndex, settings[settingIndex]);
		}
	}
}

void USBVideoSourceComponent::saveVideoSettingDefaultsFromCurrentMode()
{
	m_currentVideoSettings = std::array<float, (int)eVideoSettingType::COUNT>{};

	for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
	{
		const eVideoSettingType settingType = (eVideoSettingType)settingIndex;
		const VideoSettingConstraint& constraint = m_currentVideoConstraints[settingIndex];

		if (constraint.max_value > constraint.min_value)
		{
			const float defaultFloatvalue =
				remap_int_to_float(
					constraint.min_value, constraint.max_value,
					0.f, 1.f,
					constraint.default_value);

			setVideoSettingAsFloatFraction(
				settingType,
				defaultFloatvalue);
		}
	}

	// Update the settings entry for this video mode
	const char* modeName = m_usbVideoDevice->getVideoModeName();
	if (modeName)
	{
		USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();

		definition->setCameraSettingsForMode(modeName, m_currentVideoSettings);
		definition->notifyCameraSettingsChanged();
	}
}

void USBVideoSourceComponent::restoreVideoSettingsToCurrentMode()
{
	m_currentVideoSettings = std::array<float, (int)eVideoSettingType::COUNT>{};

	if (m_usbVideoDevice != nullptr)
	{
		const char* szVideoModeName = m_usbVideoDevice->getVideoModeName();

		if (szVideoModeName != nullptr)
		{
			// Read in current settings from the USB video device
			for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
			{
				// Fetch current settings from the USB video device
				if (float floatFraction = 0.f;
					getVideoSettingAsFloatFraction((eVideoSettingType)settingIndex, floatFraction))
				{
					m_currentVideoSettings[settingIndex] = floatFraction;
				}
				else
				{
					m_currentVideoSettings[settingIndex] = 0.f;
				}
			}

			// Apply video property settings stored in config onto the camera
			USBVideoSettingsArray videoSettings;
			USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
			if (definition->getVideoSettingsForMode(szVideoModeName, videoSettings))
			{
				for (int prop_index = 0; prop_index < (int)eVideoSettingType::COUNT; ++prop_index)
				{
					const eVideoSettingType prop_type = (eVideoSettingType)prop_index;
					const float desiredFloatValue = videoSettings[prop_index];

					setVideoSettingAsFloatFraction(prop_type, desiredFloatValue);
				}
			}
		}
	}
}

bool USBVideoSourceComponent::getVideoSettingAsFloatFraction(
	eVideoSettingType settingType,
	float& outFloatFraction) const
{
	const VideoSettingConstraint& constraint = m_currentVideoConstraints[(int)settingType];

	outFloatFraction = -1.f;

	if (m_usbVideoDevice != nullptr && constraint.max_value > constraint.min_value)
	{
		const int currentIntValue = m_usbVideoDevice->getVideoSetting(settingType);

		outFloatFraction =
			remap_int_to_float(
				constraint.min_value, constraint.max_value,
				0.f, 1.f,
				currentIntValue);

		return true;
	}

	return false;
}

bool USBVideoSourceComponent::setVideoSettingAsFloatFraction(
	eVideoSettingType settingType,
	float desiredFloatValue)
{
	const VideoSettingConstraint& constraint = m_currentVideoConstraints[(int)settingType];
	const int valueIntRange = constraint.max_value - constraint.min_value;

	if (valueIntRange > 0)
	{
		const float currentFloatValue = m_currentVideoSettings[(int)settingType];
		const float minDelta =  (float)constraint.stepping_delta / (float)valueIntRange;

		// Make sure the the desired value is different enough from the current value
		// to bother with setting it on the USB video device
		if (fabsf(currentFloatValue - desiredFloatValue) > minDelta)
		{
			const int desiredIntValue =
				remap_float_to_int(
					0.f, 1.f,
					constraint.min_value, constraint.max_value,
					desiredFloatValue);

			// Set the camera setting on the USB video device
			if (m_usbVideoDevice != nullptr)
			{
				m_usbVideoDevice->setVideoSetting(settingType, desiredIntValue);
			}

			// Remember the current setting value
			m_currentVideoSettings[(int)settingType] = desiredFloatValue;
		}

		return true;
	}

	return false;
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
		const VideoSettingConstraint& constraint = m_currentVideoConstraints[(int)property_type];

		return constraint.max_value > constraint.min_value;
	}

	return false;
}

bool USBVideoSourceComponent::setVideoSetting(const eVideoSettingType settingType, float desiredFloatValue)
{
	const VideoSettingConstraint& constraint = m_currentVideoConstraints[(int)settingType];
	const int valueIntRange = constraint.max_value - constraint.min_value;

	if (m_usbVideoDevice != nullptr && valueIntRange > 0)
	{
		const float currentFloatValue = m_currentVideoSettings[(int)settingType];
		const float minDelta = (float)constraint.stepping_delta / (float)valueIntRange;

		// Make sure the the desired value is different enough from the current value
		// to bother with setting it on the USB video device
		if (fabsf(currentFloatValue - desiredFloatValue) > minDelta)
		{
			const int desiredIntValue =
				remap_float_to_int(
					0.f, 1.f,
					constraint.min_value, constraint.max_value,
					desiredFloatValue);

			// Set the camera setting on the USB video device
			m_usbVideoDevice->setVideoSetting(settingType, desiredIntValue);
		}

		return true;
	}

	return false;
}

bool USBVideoSourceComponent::getVideoSetting(const eVideoSettingType property_type, float& outFractionValue) const
{
	if (m_usbVideoDevice != nullptr)
	{
		return getVideoSettingAsFloatFraction(property_type, outFractionValue);
	}

	return false;
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
	if (OnFrameSizeChanged)
	{
		OnFrameSizeChanged(getSelfPtr<VideoSourceComponent>());
	}
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
const std::string USBVideoSourceComponent::k_resetToDefaultsFunctionId = "reset_settings";

void USBVideoSourceComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outPropertyNames)
{
	VideoSourceComponent::getRmlFunctionDescriptors(outPropertyNames);

	outPropertyNames.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_resetToDefaultsFunctionId, "Reset to Defaults"));
}

bool USBVideoSourceComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionName = functionDesc->getFunctionName();

	if (functionName == k_resetToDefaultsFunctionId)
	{
		saveVideoSettingDefaultsFromCurrentMode();
		return true;
	}

	return VideoSourceComponent::invokeFunctionFromRml(functionDesc);
}