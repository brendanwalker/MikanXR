#include "SpoutVideoSourceComponent.h"
#include "StringUtils.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

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

void SpoutVideoSourceDefinition::setSpoutSource(const std::string& spoutSource)
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
	return StringUtils::stringify("spout://", getSpoutVideoSourceDefinition()->getSpoutSource());
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

bool SpoutVideoSourceComponent::getVideoPixelDimensions(int& outPixelWidth, int& outPixelHeight) const
{
	return true;
}

// -- IRmlPropertyInterface ----
void SpoutVideoSourceComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	VideoSourceComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			SpoutVideoSourceDefinition::k_spoutSourcePropertyId));
}

bool SpoutVideoSourceComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == SpoutVideoSourceDefinition::k_spoutSourcePropertyId)
	{
		outValue = getSpoutVideoSourceDefinition()->getSpoutSource();
		return true;
	}

	return VideoSourceComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool SpoutVideoSourceComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == SpoutVideoSourceDefinition::k_spoutSourcePropertyId)
	{
		std::string devicePath = inValue.Get<std::string>();
		getSpoutVideoSourceDefinition()->setSpoutSource(devicePath);
		return true;
	}

	return VideoSourceComponent::setPropertyValueFromRml(propertyDesc, inValue);
}