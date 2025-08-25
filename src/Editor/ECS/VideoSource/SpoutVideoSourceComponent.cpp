#include "SpoutVideoSourceComponent.h"

// -- SpoutVideoSourceDefinition ------
const std::string SpoutVideoSourceDefinition::k_spoutSourcePropertyId = "spout_source";

SpoutVideoSourceDefinition::SpoutVideoSourceDefinition()
	: VideoSourceDefinition()
{}

SpoutVideoSourceDefinition::SpoutVideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const MikanSpoutVideoSourceInfo& videoSourceInfo)
	: VideoSourceDefinition(
		videoSourceId, 
		videoSourceInfo.spout_source_name.getValue())
{}

configuru::Config SpoutVideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = VideoSourceDefinition::writeToJSON();

	pt["spout_source"] = m_spoutSource;

	return pt;
}

void SpoutVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);

	m_spoutSource = pt.get_or<std::string>("spout_source", m_spoutSource);
}

void SpoutVideoSourceDefinition::setClientSource(const std::string& spoutSource)
{
	if (spoutSource != m_spoutSource)
	{
		m_spoutSource = spoutSource;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_spoutSourcePropertyId));
	}
}

// -- SpoutVideoSourceComponent -----
SpoutVideoSourceComponent::SpoutVideoSourceComponent(MikanObjectWeakPtr owner)
	: VideoSourceComponent(owner)
{}

void SpoutVideoSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	// Close any open video source that was open
	closeVideoSource();
}

std::string SpoutVideoSourceComponent::getDevicePath() const
{
	return "";
}

std::string SpoutVideoSourceComponent::getDeviceAPI() const
{
	return "SpoutVideoSource";
}

bool SpoutVideoSourceComponent::openVideoSource()
{
	return false;
}

void SpoutVideoSourceComponent::closeVideoSource()
{

}

eVideoStreamingStatus SpoutVideoSourceComponent::startVideoStream()
{
	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus SpoutVideoSourceComponent::getVideoStreamingStatus() const
{
	return eVideoStreamingStatus::failed;
}

void SpoutVideoSourceComponent::stopVideoStream()
{}

bool SpoutVideoSourceComponent::hasNewVideoFrameAvailable(VideoFrameSection section) const
{
	return false;
}

int64_t SpoutVideoSourceComponent::readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer)
{
	return 0;
}

bool SpoutVideoSourceComponent::getPixelDimensions(int& outPixelWidth, int& outPixelHeight) const
{
	return true;
}

bool SpoutVideoSourceComponent::getCameraIntrinsics(MikanVideoSourceIntrinsics& out_camera_intrinsics) const
{
	// Not supported for Spout video sources
	return false;
}

bool SpoutVideoSourceComponent::setCameraIntrinsics(const MikanVideoSourceIntrinsics& camera_intrinsics)
{
	// Not supported for Spout video sources
	return false;
}