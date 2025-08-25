#include "NetworkVideoSourceComponent.h"
#include "MikanVideoSourceTypes.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- NetworkVideoSourceDefinition -----
const std::string NetworkVideoSourceDefinition::k_urlPropertyId = "url";

NetworkVideoSourceDefinition::NetworkVideoSourceDefinition()
	: VideoSourceDefinition()
{}

NetworkVideoSourceDefinition::NetworkVideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const MikanNetworkVideoSourceInfo& videoSourceInfo)
	: VideoSourceDefinition(
		videoSourceId, 
		videoSourceInfo.network_source_name.getValue(),
		videoSourceInfo.intrinsics)
	, m_url(videoSourceInfo.url.getValue())
{}

configuru::Config NetworkVideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = VideoSourceDefinition::writeToJSON();

	pt["url"] = m_url;

	return pt;
}

void NetworkVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);

	m_url = pt.get_or<std::string>("url", m_url);
}

void NetworkVideoSourceDefinition::setURL(const std::string& url)
{
	if (url != m_url)
	{
		m_url = url;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_urlPropertyId));
	}
}

// -- NetworkVideoSourceComponent -----
NetworkVideoSourceComponent::NetworkVideoSourceComponent(MikanObjectWeakPtr owner)
	: VideoSourceComponent(owner)
{}

void NetworkVideoSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	// Close any open video source that was open
	closeVideoSource();
}

std::string NetworkVideoSourceComponent::getDevicePath() const
{
	return "";
}

std::string NetworkVideoSourceComponent::getDeviceAPI() const
{
	return "NetworkVideoSource";
}

bool NetworkVideoSourceComponent::openVideoSource()
{
	return false;
}

void NetworkVideoSourceComponent::closeVideoSource()
{

}

eVideoStreamingStatus NetworkVideoSourceComponent::startVideoStream()
{
	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus NetworkVideoSourceComponent::getVideoStreamingStatus() const
{
	return eVideoStreamingStatus::failed;
}

void NetworkVideoSourceComponent::stopVideoStream()
{}

bool NetworkVideoSourceComponent::hasNewVideoFrameAvailable(VideoFrameSection section) const
{
	return false;
}

int64_t NetworkVideoSourceComponent::readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer)
{
	return 0;
}

void NetworkVideoSourceComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(NetworkVideoSourceDefinition::k_urlPropertyId);
}

bool NetworkVideoSourceComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (MikanComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == NetworkVideoSourceDefinition::k_urlPropertyId)
	{
		outDescriptor = {NetworkVideoSourceDefinition::k_urlPropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name};
		return true;
	}

	return false;
}

bool NetworkVideoSourceComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == NetworkVideoSourceDefinition::k_urlPropertyId)
	{
		outValue = getNetworkVideoSourceDefinition()->getURL();
		return true;
	}

	return false;
}

bool NetworkVideoSourceComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (MikanComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == NetworkVideoSourceDefinition::k_urlPropertyId)
	{
		std::string url = inValue.Get<std::string>();
		getNetworkVideoSourceDefinition()->setURL(url);
		return true;
	}

	return false;
}

const std::string NetworkVideoSourceComponent::k_calibrateIntrinsicsFunctionId = "calibrate_intrinsics";
const std::string NetworkVideoSourceComponent::k_testIntrinsicsFunctionId = "test_intrinsics";

void NetworkVideoSourceComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_calibrateIntrinsicsFunctionId);
	outPropertyNames.push_back(k_testIntrinsicsFunctionId);
}

bool NetworkVideoSourceComponent::getFunctionDescriptor(
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

bool NetworkVideoSourceComponent::invokeFunction(const std::string& functionName)
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

void NetworkVideoSourceComponent::calibrateIntrinsics()
{
	//TODO
}

void NetworkVideoSourceComponent::testIntrinsics()
{
	//TODO
}