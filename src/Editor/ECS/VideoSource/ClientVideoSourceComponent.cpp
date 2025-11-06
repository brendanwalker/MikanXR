#include "ClientSourceManager.h"
#include "ClientVideoSourceComponent.h"
#include "IEditorWindow.h"
#include "MikanVideoSourceTypes.h"
#include "MikanServer.h"
#include "VideoSourceRequestHandler.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- ClientVideoSourceDefinition ------
const std::string ClientVideoSourceDefinition::k_clientSourcePropertyId = "client_source";

ClientVideoSourceDefinition::ClientVideoSourceDefinition()
	: VideoSourceDefinition()
{}

ClientVideoSourceDefinition::ClientVideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const MikanClientVideoSourceInfo& videoSourceInfo)
	: VideoSourceDefinition(videoSourceId, videoSourceInfo.client_source_name.getValue())
{}

configuru::Config ClientVideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = VideoSourceDefinition::writeToJSON();

	pt["client_source"] = m_clientSource;

	return pt;
}

void ClientVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);

	m_clientSource = pt.get_or<std::string>("client_source", m_clientSource);
}

void ClientVideoSourceDefinition::setClientSource(const std::string& clientSource)
{
	if (clientSource != m_clientSource)
	{
		m_clientSource = clientSource;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_clientSourcePropertyId));
	}
}

// -- ClientVideoSourceComponent -----
ClientVideoSourceComponent::ClientVideoSourceComponent(MikanObjectWeakPtr owner)
	: VideoSourceComponent(owner)
{}

void ClientVideoSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	// Close any open video source that was open
	closeVideoSource();
}

IMkTexturePtr ClientVideoSourceComponent::getClientColorSourceTexture(
	eClientColorTextureType clientTextureType) const
{
	auto* clientSourceManager = getClientSourceManager();

	if (clientSourceManager != nullptr)
	{
		return clientSourceManager->getClientColorSourceTexture(getClientSourceName(), clientTextureType);
	}
}

IMkTexturePtr ClientVideoSourceComponent::getClientDepthSourceTexture(
	eClientDepthTextureType depthTextureType) const
{
	auto* clientSourceManager = getClientSourceManager();

	if (clientSourceManager != nullptr)
	{
		return clientSourceManager->getClientDepthSourceTexture(getClientSourceName(), depthTextureType);
	}
}

std::string ClientVideoSourceComponent::getDevicePath() const
{
	return getClientVideoSourceDefinition()->getClientSource();
}

std::string ClientVideoSourceComponent::getDeviceAPI() const
{
	return "ClientVideoSource";
}

bool ClientVideoSourceComponent::openVideoSource()
{
	// TODO: See if client source is available
	
	// TODO: Register for updates from client source manager
	//m_networkVideoDevice->addListener(this);

	// Apply the side effect of video mode changes 
	//notifyVideoModePropertiesChanged(m_networkVideoDevice.get());
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

void ClientVideoSourceComponent::closeVideoSource()
{
	//TODO
	//if (m_networkVideoDevice != nullptr)
	//{
	//	// Stop the video stream if it is running
	//	stopVideoStream();

	//	// Close the Network video device
	//	m_networkVideoDevice->close();

	//	// Remove the listener for the Network video device
	//	m_networkVideoDevice->removeListener(this);

	//	// Tell the Network video device manager to destroy the device
	//	auto videoSourceSystem = VideoSourceSystem::getSystem();
	//	auto networkedVideoSourceSystem = videoSourceSystem->getNetworkVideoSourceSystem();
	//	INetworkVideoDeviceManagerPtr networkVideoDeviceManager =
	//		networkedVideoSourceSystem->getNetworkVideoDeviceManager();
	//	networkVideoDeviceManager->destroyVideoDevice(m_networkVideoDevice);

	//	// Clear the Network video device pointer
	//	m_networkVideoDevice = nullptr;
	//}

	// Release any OpenCV buffer state
	releaseOpencvBufferState();

	// Let any connected clients know that the video source closed
	MikanServer::getInstance()->getVideoSourceRequestHandler()->publishVideoSourceClosedEvent();
	if (OnClosed)
	{
		OnClosed(getSelfPtr<VideoSourceComponent>());
	}
}

eVideoStreamingStatus ClientVideoSourceComponent::startVideoStream()
{
	//TODO
	//if (m_networkVideoDevice != nullptr)
	//{
	//	eVideoStreamingStatus status = m_networkVideoDevice->startVideoStream();
	//	if (status == eVideoStreamingStatus::started && OnStarted)
	//	{
	//		OnStarted(getSelfPtr<VideoSourceComponent>());
	//	}

	//	return status;
	//}

	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus ClientVideoSourceComponent::getVideoStreamingStatus() const
{
	auto* clientSourceManager = getClientSourceManager();

	if (clientSourceManager != nullptr &&
		clientSourceManager->hasClientSource(getClientSourceName()))
	{
		return eVideoStreamingStatus::started;
	}

	return eVideoStreamingStatus::failed;
}

void ClientVideoSourceComponent::stopVideoStream()
{
	//TODO
	//if (m_networkVideoDevice != nullptr)
	//{
	//	m_networkVideoDevice->stopVideoStream();

	//	if (OnStopped)
	//	{
	//		OnStopped(getSelfPtr<VideoSourceComponent>());
	//	}
	//}
}

bool ClientVideoSourceComponent::getVideoPixelDimensions(int& outPixelWidth, int& outPixelHeight) const
{
	auto* clientSourceManager = getClientSourceManager();

	if (clientSourceManager != nullptr)
	{
		return clientSourceManager->getClientSourceDimensions(getClientSourceName(), outPixelWidth, outPixelHeight);
	}

	return false;
}

// -- IClientVideoSourceListener ----
//void ClientVideoSourceComponent::notifyVideoDeviceClosed(
//	const INetworkVideoDevice* device)
//{
//	if (device == m_networkVideoDevice.get())
//	{
//		// The video source is now invalidated, so we can clear the pointer
//		// but we still want to clean up the video source state
//		m_networkVideoDevice = nullptr;
//		closeVideoSource();
//	}
//}

//void ClientVideoSourceComponent::notifyVideoFrameReceived(
//	const NetworkVideoFrameBuffer& bufferInfo)
//{
//	assert(m_networkVideoDevice != nullptr);
//
//	NetworkVideoStreamProperties streamProperties;
//	bool bVaildStream = m_networkVideoDevice->getStreamProperties(streamProperties);
//	assert(bVaildStream);
//
//	NetworkVideoSourceDefinitionPtr definition = getNetworkVideoSourceDefinition();
//	const MikanVideoSourceIntrinsics& intrinsics = definition->getCameraIntrinsics();
//
//	const bool is_frame_flipped = definition->getIsFrameMirrored();
//	const bool is_buffer_flipped = definition->getIsBufferMirrored();
//
//	// Fetch the latest video buffer frame from the device
//	if (intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
//	{
//		const auto& stereoIntrinsics = intrinsics.getStereoIntrinsics();
//		const int section_width = (int)stereoIntrinsics.pixel_width;
//		const int section_height = (int)stereoIntrinsics.pixel_height;
//
//		cv::Rect left_bounds = cv::Rect(0, 0, section_width, section_height);
//		cv::Rect right_bounds = cv::Rect(section_width, 0, section_width, section_height);
//
//		// Cache the left raw video frame
//		if (m_opencv_buffer_state[(int)VideoFrameSection::Left] != nullptr)
//		{
//			m_opencv_buffer_state[(int)VideoFrameSection::Left]->writeStereoVideoFrameSection(
//				bufferInfo.data,
//				is_buffer_flipped ? right_bounds : left_bounds,
//				is_frame_flipped);
//		}
//
//		// Cache the right raw video frame
//		if (m_opencv_buffer_state[(int)VideoFrameSection::Right] != nullptr)
//		{
//			m_opencv_buffer_state[(int)VideoFrameSection::Right]->writeStereoVideoFrameSection(
//				bufferInfo.data,
//				is_buffer_flipped ? left_bounds : right_bounds,
//				is_frame_flipped);
//		}
//	}
//	else
//	{
//		// Cache the raw video frame
//		if (m_opencv_buffer_state[(int)VideoFrameSection::Primary] != nullptr)
//		{
//			m_opencv_buffer_state[(int)VideoFrameSection::Primary]->writeVideoFrame(
//				bufferInfo.data, is_frame_flipped);
//		}
//	}
//}

// -- IRmlPropertyInterface ----
void ClientVideoSourceComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	VideoSourceComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			ClientVideoSourceDefinition::k_clientSourcePropertyId));
}

bool ClientVideoSourceComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == ClientVideoSourceDefinition::k_clientSourcePropertyId)
	{
		outValue = getClientVideoSourceDefinition()->getClientSource();
		return true;
	}

	return VideoSourceComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool ClientVideoSourceComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == ClientVideoSourceDefinition::k_clientSourcePropertyId)
	{
		std::string devicePath = inValue.Get<std::string>();
		getClientVideoSourceDefinition()->setClientSource(devicePath);
		return true;
	}

	return VideoSourceComponent::setPropertyValueFromRml(propertyDesc, inValue);
}

ClientSourceManager* ClientVideoSourceComponent::getClientSourceManager() const
{
	return getOwnerEditorWindow()->getClientSourceManager();
}

const std::string& ClientVideoSourceComponent::getClientSourceName() const
{
	return getClientVideoSourceDefinition()->getClientSource();
}