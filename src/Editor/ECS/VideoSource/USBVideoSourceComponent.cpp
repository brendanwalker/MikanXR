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

#include <easy/profiler.h>

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
	writeStdArrayMap<float, (int)VideoPropertyType::COUNT>(pt, "cameraSettings", m_cameraSettingsMap);

	return pt;
}

void USBVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);
	m_devicePath = pt.get_or<std::string>("device_path", m_devicePath);
	m_videoMode = pt.get_or<std::string>("video_mode", m_videoMode);
	readStdArrayMap<float, (int)VideoPropertyType::COUNT>(pt, "cameraSettings", m_cameraSettingsMap);
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

bool USBVideoSourceDefinition::getCameraSettingValue(
	const std::string& modeName,
	eUsbCameraSettingType settingType,
	float& outValue) const
{
	if (m_cameraSettingsMap.find(modeName) != m_cameraSettingsMap.end())
	{
		const auto& settings = m_cameraSettingsMap.at(modeName);
		
		outValue= settings[(int)settingType];
		return true; // Found the setting value
	}

	return false; // Default value if not found
}

void USBVideoSourceDefinition::setCameraSettingValue(
	const std::string& modeName,
	eUsbCameraSettingType settingType,
	float value,
	bool bBroadcastMarkDirty)
{
	bool bIsDirty = false;

	auto it = m_cameraSettingsMap.find(modeName);
	if (it == m_cameraSettingsMap.end())
	{
		 auto settings = std::array<float, (int)eUsbCameraSettingType::COUNT>();

		 settings[(int)settingType] = value;
		 m_cameraSettingsMap[modeName] = settings;
		 bIsDirty = true;
	}
	else
	{
		auto& settings = it->second;

		if (settings[(int)settingType] != value)
		{
			settings[(int)settingType] = value;
			bIsDirty = true;
		}
	}

	if (bIsDirty && bBroadcastMarkDirty)
	{
		markCameraSettingsDirty();
	}
}

void USBVideoSourceDefinition::markCameraSettingsDirty()
{
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_cameraSettingsPropertyId));
}

// -- USBVideoSourceComponent -----
USBVideoSourceComponent::USBVideoSourceComponent(MikanObjectWeakPtr owner)
	: VideoSourceComponent(owner)
	, m_lastVideoFrameReadIndex(0)
	, m_usbVideoDevice(nullptr)
	, m_projectionMatrix(glm::mat4(1.f))
{
	for (int i = 0; i < MAX_PROJECTION_COUNT; ++i)
	{
		m_opencv_buffer_state[i] = nullptr;
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
	for (int prop_index = 0; prop_index < (int)eUsbCameraSettingType::COUNT; ++prop_index)
	{
		const eUsbCameraSettingType prop_type = (eUsbCameraSettingType)prop_index;
		
		if (UsbCameraSettingConstraint constraint;
			m_usbVideoDevice->getCameraSettingConstraint(prop_type, constraint) &&
			constraint.is_supported)
		{
			float desiredFloatValue = 0.f;
			if (definition->getCameraSettingValue(
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
				m_usbVideoDevice->setCameraSetting(prop_type, desiredIntValue);
			}
			else
			{
				const int currentIntValue= m_usbVideoDevice->getCameraSetting(prop_type);
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
		definition->markCameraSettingsDirty();
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

static eVideoStreamingStatus usbStatusToVideoStatus(eUsbVideoStreamingStatus usbStatus)
{
	switch (usbStatus)
	{
	case eUsbVideoStreamingStatus::failed:
		return eVideoStreamingStatus::failed;
	case eUsbVideoStreamingStatus::pendingStart:
		return eVideoStreamingStatus::pendingStart;
	case eUsbVideoStreamingStatus::started:
		return eVideoStreamingStatus::started;
	case eUsbVideoStreamingStatus::stopped:
		return eVideoStreamingStatus::stopped;
	default:
		return eVideoStreamingStatus::failed;
	}
}

eVideoStreamingStatus USBVideoSourceComponent::startVideoStream()
{
	if (m_usbVideoDevice != nullptr)
	{
		auto status= usbStatusToVideoStatus(m_usbVideoDevice->startVideoStream());
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
		return usbStatusToVideoStatus(m_usbVideoDevice->getVideoStreamingStatus());
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

bool USBVideoSourceComponent::hasNewVideoFrameAvailable(VideoFrameSection section) const
{
	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	const MikanVideoSourceIntrinsics& intrinsics = definition->getCameraIntrinsics();

	int64_t lastFrameWriteIndex = 0;

	if (intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
	{
		if ((section == VideoFrameSection::Left || section == VideoFrameSection::Right) &&
			m_opencv_buffer_state[(int)section] != nullptr)
		{
			lastFrameWriteIndex = m_opencv_buffer_state[(int)section]->getLastVideoFrameWriteIndex();
		}
	}
	else
	{
		if (section == VideoFrameSection::Primary &&
			m_opencv_buffer_state[(int)VideoFrameSection::Primary] != nullptr)
		{
			lastFrameWriteIndex = m_opencv_buffer_state[(int)VideoFrameSection::Primary]->getLastVideoFrameWriteIndex();
		}
	}

	return lastFrameWriteIndex != m_lastVideoFrameReadIndex;
}

int64_t USBVideoSourceComponent::readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer)
{
	EASY_FUNCTION();

	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	const MikanVideoSourceIntrinsics& intrinsics = definition->getCameraIntrinsics();

	if (intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
	{
		if ((section == VideoFrameSection::Left || section == VideoFrameSection::Right) &&
			m_opencv_buffer_state[(int)section] != nullptr)
		{
			m_lastVideoFrameReadIndex =
				m_opencv_buffer_state[(int)section]->readVideoFrame(
					outBuffer,
					m_lastVideoFrameReadIndex);
		}
	}
	else
	{
		if (section == VideoFrameSection::Primary &&
			m_opencv_buffer_state[(int)VideoFrameSection::Primary] != nullptr)
		{
			m_lastVideoFrameReadIndex =
				m_opencv_buffer_state[(int)VideoFrameSection::Primary]->readVideoFrame(
					outBuffer,
					m_lastVideoFrameReadIndex);
		}
	}

	return m_lastVideoFrameReadIndex;
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

bool USBVideoSourceComponent::reallocateOpencvBufferState()
{
	releaseOpencvBufferState();

	if (m_usbVideoDevice == nullptr)
		return false;

	int videoModeIndex= m_usbVideoDevice->getVideoModeIndex();
	if (videoModeIndex < 0)
		return false;

	UsbVideoModeProperties videoModeProperties;
	if (!m_usbVideoDevice->getVideoModeProperties(videoModeIndex, videoModeProperties))
		return false;

	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	const MikanVideoSourceIntrinsics& intrinsics= definition->getCameraIntrinsics();

	// Allocate the OpenCV scratch buffers used for finding tracking blobs
	if (intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
	{		
		const MikanStereoIntrinsics& stereoIntrinsics= intrinsics.getStereoIntrinsics();
		
		m_opencv_buffer_state[(int)VideoFrameSection::Left] =
			new OpenCVVideoFrameBuffer(
				videoModeProperties.width, videoModeProperties.height,
				stereoIntrinsics.pixel_width, stereoIntrinsics.pixel_width,
				VideoFrameSection::Left);
		m_opencv_buffer_state[(int)VideoFrameSection::Right] =
			new OpenCVVideoFrameBuffer(
				videoModeProperties.width, videoModeProperties.height,
				stereoIntrinsics.pixel_width, stereoIntrinsics.pixel_width,
				VideoFrameSection::Right);
	}
	else
	{
		const MikanMonoIntrinsics& monoIntrinsics = intrinsics.getMonoIntrinsics();

		m_opencv_buffer_state[(int)VideoFrameSection::Primary] =
			new OpenCVVideoFrameBuffer(
				videoModeProperties.width, videoModeProperties.height,
				monoIntrinsics.pixel_width, monoIntrinsics.pixel_width,
				VideoFrameSection::Primary);
	}

	return true;
}

void USBVideoSourceComponent::releaseOpencvBufferState()
{
	// Delete any existing OpenCV buffers
	for (int i = 0; i < MAX_PROJECTION_COUNT; ++i)
	{
		if (m_opencv_buffer_state[i] != nullptr)
		{
			delete m_opencv_buffer_state[i];
			m_opencv_buffer_state[i] = nullptr;
		}
	}
}

void USBVideoSourceComponent::recomputeCameraProjectionMatrix()
{
	USBVideoSourceDefinitionPtr definition = getUSBVideoSourceDefinition();
	const MikanVideoSourceIntrinsics& intrinsics = definition->getCameraIntrinsics();

	switch (intrinsics.intrinsics_type)
	{
	case MONO_CAMERA_INTRINSICS:
	{
		const MikanMonoIntrinsics& monoIntrinsics = intrinsics.getMonoIntrinsics();

		computeOpenGLProjMatFromCameraIntrinsics(
			monoIntrinsics,
			m_projectionMatrix);
	} break;
	case STEREO_CAMERA_INTRINSICS:
	{
		const MikanStereoIntrinsics& stereoIntrinsics = intrinsics.getStereoIntrinsics();

		computeOpenGLProjMatFromCameraIntrinsics(
			stereoIntrinsics,
			eStereoIntrinsicsSide::left,
			m_projectionMatrix);
	} break;
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

void USBVideoSourceComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(USBVideoSourceDefinition::k_devicePathPropertyId);
	outPropertyNames.push_back(USBVideoSourceDefinition::k_videoModePropertyId);
	outPropertyNames.push_back(USBVideoSourceDefinition::k_cameraSettingsPropertyId);
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
	// TODO Camera settings
	//else if (propertyName == USBVideoSourceDefinition::k_cameraSettingsPropertyId)
	//{
	//	outDescriptor = {USBVideoSourceDefinition::k_cameraSettingsPropertyId, ePropertyDataType::datatype_float, ePropertySemantic::size1d};
	//	return true;
	//}

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
	// TODO Camera settings
	//else if (propertyName == USBVideoSourceDefinition::k_brightnessPropertyId)
	//{
	//	outValue = getUSBVideoSourceDefinition()->getBrightness();
	//	return true;
	//}
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
		setVideoModeByName(videoMode);
		return true;
	}
	// TODO Camera settings
	//else if (propertyName == USBVideoSourceDefinition::k_brightnessPropertyId)
	//{
	//	float brightness = inValue.Get<float>();
	//	getUSBVideoSourceDefinition()->setBrightness(brightness);
	//	return true;
	//}

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