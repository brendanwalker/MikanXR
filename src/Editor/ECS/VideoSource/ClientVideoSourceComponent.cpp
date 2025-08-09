#include "ClientVideoSourceComponent.h"
#include "MikanVideoSourceTypes.h"

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

bool ClientVideoSourceComponent::openVideoSource()
{
	return false;
}

void ClientVideoSourceComponent::closeVideoSource()
{

}

eVideoStreamingStatus ClientVideoSourceComponent::startVideoStream()
{
	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus ClientVideoSourceComponent::getVideoStreamingStatus() const
{
	return eVideoStreamingStatus::failed;
}

void ClientVideoSourceComponent::stopVideoStream()
{}

bool ClientVideoSourceComponent::hasNewVideoFrameAvailable(VideoFrameSection section) const
{
	return false;
}

int64_t ClientVideoSourceComponent::readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer)
{
	return 0;
}

bool ClientVideoSourceComponent::getPixelDimensions(int& outPixelWidth, int& outPixelHeight) const
{
	return true;
}

bool ClientVideoSourceComponent::getCameraIntrinsics(MikanVideoSourceIntrinsics& out_camera_intrinsics) const
{
	// No camera intrinsics for client video sources
	return false;
}

bool ClientVideoSourceComponent::setCameraIntrinsics(const MikanVideoSourceIntrinsics& camera_intrinsics)
{
	// No camera intrinsics for client video sources
	return false;
}