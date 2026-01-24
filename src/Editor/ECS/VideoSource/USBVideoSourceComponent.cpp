#include "CameraMath.h"
#include "Logger.h"
#include "MathUtility.h"
#include "MikanServer.h"
#include "MikanObject.h"
#include "OpenCVVideoFrameBuffer.h"
#include "ThreadUtils.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceRequestHandler.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include <assert.h>

#define DEFAULT_DESIRED_VIDEO_WIDTH			1280
#define DEFAULT_DESIRED_VIDEO_HEIGHT		720
#define DEFAULT_DESIRED_FRAME_RATE			30

// -- USBVideoSourceDefinition -----
const std::string USBVideoSourceDefinition::k_desiredDevicePathPropertyId = "desired_device_path";
const std::string USBVideoSourceDefinition::k_videoModePropertyId = "video_mode";
const std::string USBVideoSourceDefinition::k_videoSettingsPropertyId = "video_settings";

USBVideoSourceDefinition::USBVideoSourceDefinition()
	: VideoSourceDefinition()
	, m_devicePath("")
	, m_videoMode("")
{	
}

USBVideoSourceDefinition::USBVideoSourceDefinition(
	MikanVideoSourceID videoSourceId)
	: VideoSourceDefinition(videoSourceId)
	, m_devicePath("")
	, m_videoMode("")
{
}

bool USBVideoSourceDefinition::wantsSaveForPropertyChange(
	const ConfigPropertyChangeSet& changedPropertySet) const 
{
	// Current device path is read-only, don't trigger save on change
	// TODO: All read-only properties should be ignored for save triggers
	if (changedPropertySet.hasPropertyName(USBVideoSourceComponent::k_currentDevicePathPropertyId) &&
		changedPropertySet.getSet().size() == 1)
	{
		return false;
	}

	return VideoSourceDefinition::wantsSaveForPropertyChange(changedPropertySet);
}

configuru::Config USBVideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = VideoSourceDefinition::writeToJSON();

	pt[k_desiredDevicePathPropertyId] = m_devicePath;
	pt[k_videoModePropertyId] = m_videoMode;
	writeStdArrayMap<float, (int)eVideoSettingType::COUNT>(pt, k_videoSettingsPropertyId, m_videoSettingsMap);

	return pt;
}

void USBVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);
	m_devicePath = pt.get_or<std::string>(k_desiredDevicePathPropertyId, m_devicePath);
	m_videoMode = pt.get_or<std::string>(k_videoModePropertyId, m_videoMode);
	readStdArrayMap<float, (int)eVideoSettingType::COUNT>(pt, k_videoSettingsPropertyId, m_videoSettingsMap);
}

void USBVideoSourceDefinition::setDevicePath(const std::string& devicePath)
{
	if (devicePath != m_devicePath)
	{
		m_devicePath = devicePath;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_desiredDevicePathPropertyId));
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
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_videoSettingsPropertyId));
}

// -- USBVideoSourceComponent -----
USBVideoSourceComponent::USBVideoSourceComponent(MikanObjectWeakPtr owner)
	: VideoSourceComponent(owner)
	, m_usbVideoDevice(nullptr)
{
	m_currentVideoSettings = {};
	m_currentVideoConstraints = {};
	m_bWantsUpdate = true;
}

// -- IEntityAccessor ----
rfk::Struct const* USBVideoSourceComponent::getClientAPIValuesStructType() const
{
	return &MikanUSBVideoSourceValues::staticGetArchetype();
}

void USBVideoSourceComponent::init()
{
	MikanComponent::init();

	// Attempt to open the video source during update (may fail if device not connected)
	m_bDeviceChanged = true;
}

void USBVideoSourceComponent::update(float deltaSeconds)
{
	VideoSourceComponent::update(deltaSeconds);

	// If the device path changed, reopen the video source
	if (m_bDeviceChanged)
	{
		m_bDeviceChanged = false;

		closeVideoSource();
		openVideoSource();
	}
	// If the video mode changed, update the video mode
	else if (m_bModeChanged)
	{
		m_bModeChanged = false;

		if (updateVideoMode())
		{
			handleVideoModeUpdated();
		}
	}
	// If camera settings changed, update the camera settings
	else if (m_bSettingsChanged)
	{
		m_bSettingsChanged = false;
		handleVideoModeSettingUpdated();
	}
	else if (m_bWantsStreamActive)
	{
		handleWantsActiveStream();
	}
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

	// Signal to the RmlModel to rebuild the video mode list
	getDefinition()->notifyPropertyChanged(
		ConfigPropertyChangeSet().addPropertyName(k_currentDevicePathPropertyId));

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
	if (changedPropertySet.hasPropertyName(USBVideoSourceDefinition::k_desiredDevicePathPropertyId))
	{
		m_bDeviceChanged = true;
	}
	// If the video mode changed, update the video mode
	else if (changedPropertySet.hasPropertyName(USBVideoSourceDefinition::k_videoModePropertyId))
	{
		m_bModeChanged = true;
	}
	// If camera settings changed, update the camera settings
	else if (changedPropertySet.hasPropertyName(USBVideoSourceDefinition::k_videoSettingsPropertyId))
	{
		m_bSettingsChanged = true;
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
	std::string desiredVideoMode = definition->getVideoMode();
	const char* szCurrentVideoMode = m_usbVideoDevice->getVideoModeName();
	std::string currentVideoMode = szCurrentVideoMode ? szCurrentVideoMode : "";
	if (desiredVideoMode != currentVideoMode || currentVideoMode.empty())
	{
		bool bHasValidMode = !currentVideoMode.empty();

		// If we have a desired video mode, try to set it first
		if (!desiredVideoMode.empty())
		{
			if (m_usbVideoDevice->setVideoModeByName(desiredVideoMode.c_str()))
			{
				currentVideoMode = desiredVideoMode;
				bHasValidMode = true;
			}
		}

		// If we don't have a valid desired video mode yet, try to set a default one
		if (!bHasValidMode)
		{
			const int videoFormatIndex =
				findBestVideoModeIndex(
					DEFAULT_DESIRED_VIDEO_WIDTH,
					DEFAULT_DESIRED_VIDEO_HEIGHT,
					DEFAULT_DESIRED_FRAME_RATE);
			if (videoFormatIndex != -1 && m_usbVideoDevice->setVideoModeByIndex(videoFormatIndex))
			{
				currentVideoMode = m_usbVideoDevice->getVideoModeName();
				bHasValidMode = true;
			}
		}

		// If we succeeded in setting a valid video mode, 
		// update the definition to track which mode is currently set
		if (bHasValidMode)
		{
			definition->setVideoMode(currentVideoMode);
		}
	}

	return true;
}

int USBVideoSourceComponent::findBestVideoModeIndex(
	int w,
	int h,
	int frameRate) const
{
	assert(m_usbVideoDevice != nullptr);
	int result_id = -1;

	size_t numFormats= m_usbVideoDevice->getAvailableVideoModesCount();
	if (numFormats > 0)
	{
		for (int attempt = 0; attempt < 2; ++attempt)
		{
			for (size_t testDeviceIndex = 0; testDeviceIndex < numFormats; ++testDeviceIndex)
			{
				if (UsbVideoModeProperties modeInfo;
					m_usbVideoDevice->getVideoModeProperties(testDeviceIndex, modeInfo) &&
					modeInfo.frame_rate_demonenator > 0)
				{
					int rounded_frame_rate = modeInfo.frame_rate_numerator / modeInfo.frame_rate_demonenator;

					if ((w == -1 || modeInfo.width == w) &&
						(h == -1 || modeInfo.height == h) &&
						(frameRate == -1 || rounded_frame_rate == frameRate))
					{
						result_id = (int)testDeviceIndex;
						break;
					}
				}
			}

			if (result_id != -1)
			{
				break;
			}
			else if (attempt == 0)
			{
				// Fallback to no FPS restriction on second pass
				frameRate = -1;
			}
		}

		if (result_id == -1)
		{
			// If we didn't find an exact match, just return the first available mode
			result_id = 0;
		}
	}

	return result_id;
}

bool USBVideoSourceComponent::handleVideoModeUpdated()
{
	if (m_usbVideoDevice == nullptr)
		return false;

	const char* szVideoModeName = m_usbVideoDevice->getVideoModeName();
	if (szVideoModeName == nullptr)
		return false;

	// Try and re-open the device if it is not already open
	if (!m_usbVideoDevice->getIsOpen())
	{
		if (!m_usbVideoDevice->open())
		{
			return false;
		}
	}

	// Cache the video setting constraints for the new video mode first
	// (needed by restoreVideoSettingsToCurrentMode / backupVideoSettingsFromCurrentMode)
	for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
	{
		const eVideoSettingType settingType = (eVideoSettingType)settingIndex;
		VideoSettingConstraint& constraint = m_currentVideoConstraints[settingIndex];

		m_usbVideoDevice->getVideoSettingConstraint(settingType, constraint);

		if (settingType == eVideoSettingType::Gain && constraint.default_value == 0)
		{
			// Some cameras report Gain default value as 0, 
			// Which is invalid, so we set it to mid-range instead
			constraint.default_value = (constraint.min_value + constraint.max_value) / 2;
		}
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

void USBVideoSourceComponent::handleWantsActiveStream()
{
	if (m_usbVideoDevice != nullptr &&
		m_usbVideoDevice->getIsOpen())
	{
		eVideoStreamingStatus status = m_usbVideoDevice->getVideoStreamingStatus();
		switch (status)
		{
		case eVideoStreamingStatus::started:
			if (m_bPendingStartStream)
			{
				m_bPendingStartStream = false;

				// Successfully started streaming!
				if (OnStarted)
				{
					OnStarted(getSelfPtr<VideoSourceComponent>());
				}
			}
			break;
		case eVideoStreamingStatus::pendingStart:
			// Still starting up, do nothing
			break;
		case eVideoStreamingStatus::failed:
		case eVideoStreamingStatus::stopped:
			{
				m_usbVideoDevice->startVideoStream();
				m_bPendingStartStream = true;
			}
			break;
		}
	}
	else
	{
		// Device closed before we could start streaming
		m_bWantsStreamActive = false;
		m_bPendingStartStream = false;
	}
}

void USBVideoSourceComponent::saveVideoSettingDefaultsFromCurrentMode()
{
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
				defaultFloatvalue, 
				true); // force apply the default value even if it matches current cached value
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
				for (int settingIndex = 0; settingIndex < (int)eVideoSettingType::COUNT; ++settingIndex)
				{
					const eVideoSettingType settingType = (eVideoSettingType)settingIndex;
					const float desiredFloatValue = videoSettings[settingIndex];

					setVideoSettingAsFloatFraction(
						settingType, 
						desiredFloatValue, 
						true); // force apply the default value even if it matches current cached value
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
	float desiredFloatValue, 
	bool bForce)
{
	const VideoSettingConstraint& constraint = m_currentVideoConstraints[(int)settingType];
	const int valueIntRange = constraint.max_value - constraint.min_value;

	if (valueIntRange > 0)
	{
		const float currentFloatValue = m_currentVideoSettings[(int)settingType];
		const float minDelta =  (float)constraint.stepping_delta / (float)valueIntRange;

		// Make sure the the desired value is different enough from the current value
		// to bother with setting it on the USB video device
		if (bForce || fabsf(currentFloatValue - desiredFloatValue) > minDelta)
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

			// Update the cached setting value
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
		// Let any connected clients know that the video source closed
		MikanServer::getInstance()->getVideoSourceRequestHandler()->publishVideoSourceClosedEvent();
		if (OnClosed)
		{
			OnClosed(getSelfPtr<VideoSourceComponent>());
		}

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
}

eVideoStreamingStatus USBVideoSourceComponent::startVideoStream()
{
	if (m_usbVideoDevice != nullptr)
	{
		m_bWantsStreamActive = true;
		m_bPendingStartStream = true;

		handleWantsActiveStream();

		return m_usbVideoDevice->getVideoStreamingStatus();
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
		m_bWantsStreamActive = false;
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
	return setVideoSettingAsFloatFraction(settingType, desiredFloatValue);
}

bool USBVideoSourceComponent::getVideoSetting(const eVideoSettingType settingType, float& outFractionValue) const
{
	if (m_usbVideoDevice != nullptr)
	{
		const VideoSettingConstraint& constraint = m_currentVideoConstraints[(int)settingType];

		// Only return valid settings
		if (constraint.max_value > constraint.min_value)
		{
			// Return the cached current setting value
			outFractionValue = m_currentVideoSettings[(int)settingType];
			return true;
		}
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

	// Convert video buffer to BGR format if needed
	cv::Mat bgrMat;
	if (bufferInfo.data_format == eUSBVideoFrameBufferFormat::USBVideo_NV12)
	{
		// NV12 layout: [Y plane: width × height] [UV plane: width × height/2]
		// Always use section-based approach (handles inter-plane padding correctly)
		if (bufferInfo.section_count == 2)
		{
			const UsbVideoFrameSection& ySection = bufferInfo.sections[0];
			const UsbVideoFrameSection& uvSection = bufferInfo.sections[1];

			// Create separate Mat views for Y and UV planes using section offsets
			cv::Mat yPlane(
				ySection.pixel_height,
				ySection.pixel_width,
				CV_8UC1,
				(void*)(bufferInfo.data + ySection.start_offset),
				ySection.stride);

			// For NV12, UV plane is interleaved U and V bytes
			// When using CV_8UC2, width is half of Y plane width (each pixel = 2 bytes)
			cv::Mat uvPlane(
				uvSection.pixel_height,
				uvSection.pixel_width / 2,  // Half width since each pixel is 2 bytes (U+V)
				CV_8UC2,
				(void*)(bufferInfo.data + uvSection.start_offset),
				uvSection.stride);

			// Use cvtColorTwoPlane for proper NV12 conversion with separate planes
			cv::cvtColorTwoPlane(yPlane, uvPlane, bgrMat, cv::COLOR_YUV2BGR_NV12);
		}
		else
		{
			// Should not happen, but provide fallback
			const UsbVideoFrameSection& section = bufferInfo.sections[0];
			bgrMat = cv::Mat(section.pixel_height, section.pixel_width, CV_8UC3, cv::Scalar(0, 0, 0));
		}
	}
	else if (bufferInfo.data_format == eUSBVideoFrameBufferFormat::USBVideo_YUY2)
	{
		// YUY2 (YUYV) format: 4:2:2 packed format (2 bytes per pixel)
		const UsbVideoFrameSection& section = bufferInfo.sections[0];
		cv::Mat yuy2Mat(
			section.pixel_height,
			section.pixel_width,
			CV_8UC2,
			(void*)(bufferInfo.data + section.start_offset),
			section.stride);  // stride is already in bytes per row
		cv::cvtColor(yuy2Mat, bgrMat, cv::COLOR_YUV2BGR_YUY2);
	}
	else if (bufferInfo.data_format == eUSBVideoFrameBufferFormat::USBVideo_RGB24)
	{
		// RGB24 format: convert to BGR
		const UsbVideoFrameSection& section = bufferInfo.sections[0];
		cv::Mat rgb24Mat(
			section.pixel_height,
			section.pixel_width,
			CV_8UC3,
			(void*)(bufferInfo.data + section.start_offset),
			section.stride);  // stride is already in bytes per row
		cv::cvtColor(rgb24Mat, bgrMat, cv::COLOR_RGB2BGR);
	}
	else
	{
		// Unknown format - create empty mat as fallback
		if (bufferInfo.section_count > 0)
		{
			const UsbVideoFrameSection& section = bufferInfo.sections[0];
			bgrMat = cv::Mat(section.pixel_height, section.pixel_width, CV_8UC3, cv::Scalar(0, 0, 0));
		}
		else
		{
			bgrMat = cv::Mat(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));  // Default fallback size
		}
	}

	// Now bgrMat contains the BGR image data that writeVideoFrame expects
	const unsigned char* bgrData = bgrMat.data;

	// Fetch the latest video buffer frame from the device
	if (intrinsics.intrinsics_type == MikanIntrinsicsType::STEREO_CAMERA_INTRINSICS)
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
				bgrData,
				is_buffer_flipped ? right_bounds : left_bounds,
				is_frame_flipped);
		}

		// Cache the right raw video frame
		if (m_opencv_buffer_state[(int)VideoFrameSection::Right] != nullptr)
		{
			m_opencv_buffer_state[(int)VideoFrameSection::Right]->writeStereoVideoFrameSection(
				bgrData,
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
				bgrData, is_frame_flipped);
		}
	}
}

// -- IPropertyInterface ----
const std::string USBVideoSourceComponent::k_currentDevicePathPropertyId = "current_device_path";

void USBVideoSourceComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	VideoSourceComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			USBVideoSourceDefinition::k_desiredDevicePathPropertyId, MikanVariantType::STRING));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			USBVideoSourceComponent::k_currentDevicePathPropertyId, MikanVariantType::STRING)
		->setReadOnly());
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			USBVideoSourceDefinition::k_videoModePropertyId, MikanVariantType::STRING));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			USBVideoSourceDefinition::k_videoSettingsPropertyId, MikanVariantType::FLOAT_ARRAY)
		->setReadOnly());
}

bool USBVideoSourceComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == USBVideoSourceDefinition::k_desiredDevicePathPropertyId)
	{
		outValue = getUSBVideoSourceDefinition()->getDevicePath();
		return true;
	}
	else if (propertyName == USBVideoSourceComponent::k_currentDevicePathPropertyId)
	{
		outValue = getDevicePath();
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_videoModePropertyId)
	{
		outValue = getUSBVideoSourceDefinition()->getVideoMode();
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_videoSettingsPropertyId)
	{
		outValue = std::vector<float>(m_currentVideoSettings.begin(), m_currentVideoSettings.end());
		return true;
	}

	return VideoSourceComponent::getPropertyValue(propertyName, outValue);
}

bool USBVideoSourceComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == USBVideoSourceDefinition::k_desiredDevicePathPropertyId)
	{
		std::string devicePath = inValue.getStringValue();
		getUSBVideoSourceDefinition()->setDevicePath(devicePath);
		return true;
	}
	else if (propertyName == USBVideoSourceDefinition::k_videoModePropertyId)
	{
		std::string videoMode = inValue.getStringValue();
		setVideoModeByName(videoMode);
		return true;
	}

	return VideoSourceComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
const std::string USBVideoSourceComponent::k_resetToDefaultsFunctionId = "reset_settings";

void USBVideoSourceComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outPropertyNames)
{
	VideoSourceComponent::getFunctionDescriptors(outPropertyNames);

	outPropertyNames.push_back(
		std::make_shared<FunctionDescriptor>(
			k_resetToDefaultsFunctionId, "Reset to Defaults"));
}

bool USBVideoSourceComponent::invokeFunction(FunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionName = functionDesc->getFunctionName();

	if (functionName == k_resetToDefaultsFunctionId)
	{
		saveVideoSettingDefaultsFromCurrentMode();
		return true;
	}

	return VideoSourceComponent::invokeFunction(functionDesc);
}