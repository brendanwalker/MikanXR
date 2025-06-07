#include "App.h"
#include "VideoSourceSystemConfig.h"
#include "BoxColliderComponent.h"
#include "TransformComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- VideoSourceDefinition -----
VideoSourceDefinition::VideoSourceDefinition()
	: MikanComponentDefinition()
	, m_videoSourceId(INVALID_MIKAN_ID)
{}

VideoSourceDefinition::VideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const std::string& videoSourceName) 
	: MikanComponentDefinition(videoSourceName)
	, m_videoSourceId(videoSourceId)
{}

configuru::Config VideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();
	
	pt["video_source_id"] = m_videoSourceId;

	return pt;
}

void VideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_videoSourceId = pt.get_or<MikanVideoSourceID>("video_source_id", m_videoSourceId);
}

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
	const std::string& videoSourceName) 
	: VideoSourceDefinition(videoSourceId, videoSourceName)
	, m_devicePath("")
	, m_videoMode("")
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

// -- NetworkVideoSourceDefinition -----
const std::string NetworkVideoSourceDefinition::k_urlPropertyId= "url";

NetworkVideoSourceDefinition::NetworkVideoSourceDefinition() 
	: VideoSourceDefinition() 
{
}

NetworkVideoSourceDefinition::NetworkVideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const std::string& videoSourceName)
	: VideoSourceDefinition(videoSourceId, videoSourceName)
{

}

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

// -- SpoutVideoSourceDefinition ------
const std::string SpoutVideoSourceDefinition::k_spoutSourcePropertyId = "spout_source";
const std::string SpoutVideoSourceDefinition::k_syncWithCameraPropertyId = "sync_with_camera";

SpoutVideoSourceDefinition::SpoutVideoSourceDefinition()
	: VideoSourceDefinition()
{
}

SpoutVideoSourceDefinition::SpoutVideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const std::string& videoSourceName)
	: VideoSourceDefinition(videoSourceId, videoSourceName)
{
}

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
	m_bSyncWithCamera = pt.get_or<bool>("sync_with_camera", m_bSyncWithCamera);
}

void SpoutVideoSourceDefinition::setSpoutSource(const std::string& spoutSource)
{
	if (spoutSource != m_spoutSource)
	{
		m_spoutSource = spoutSource;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_spoutSourcePropertyId));
	}
}

void SpoutVideoSourceDefinition::setSyncWithCamera(bool bSyncFlag)
{
	if (bSyncFlag != m_bSyncWithCamera)
	{
		m_bSyncWithCamera = bSyncFlag;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_syncWithCameraPropertyId));
	}
}

// -- VideoSourceSystemConfig -----
const std::string VideoSourceSystemConfig::k_usbVideoSourceListPropertyId= "usbVideoSourceList";
const std::string VideoSourceSystemConfig::k_networkedVideoSourceListPropertyId= "networkVideoSourceList";
const std::string VideoSourceSystemConfig::k_spoutVideoSourceListPropertyId= "spoutVideoSourceList";

configuru::Config VideoSourceSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["next_video_source_id"] = m_nextVideoSourceId;

	std::vector<configuru::Config> usbVideoSourceConfigs;
	for (auto videoSource : m_usbVideoSourceList)
	{
		usbVideoSourceConfigs.push_back(videoSource->writeToJSON());
	}
	pt.insert_or_assign(std::string("usb_video_sources"), usbVideoSourceConfigs);

	std::vector<configuru::Config> networkedVideoSourceConfigs;
	for (auto videoSource : m_networkedVideoSourceList)
	{
		networkedVideoSourceConfigs.push_back(videoSource->writeToJSON());
	}
	pt.insert_or_assign(std::string("networked_video_sources"), networkedVideoSourceConfigs);

	std::vector<configuru::Config> spoutVideoSourceConfigs;
	for (auto videoSource : m_spoutVideoSourceList)
	{
		spoutVideoSourceConfigs.push_back(videoSource->writeToJSON());
	}
	pt.insert_or_assign(std::string("spout_video_sources"), spoutVideoSourceConfigs);

	return pt;
}

void VideoSourceSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextVideoSourceId = pt.get_or<int>("next_video_source_id", m_nextVideoSourceId);

	// Read in the usb video sources
	m_usbVideoSourceList.clear();
	if (pt.has_key("usb_video_sources"))
	{
		for (const configuru::Config& videoSource_pt : pt["usb_video_sources"].as_array())
		{
			auto definitionPtr = std::make_shared<USBVideoSourceDefinition>();

			definitionPtr->readFromJSON(videoSource_pt);
			m_usbVideoSourceList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}

	// Read in the networked video sources
	m_networkedVideoSourceList.clear();
	if (pt.has_key("networked_video_sources"))
	{
		for (const configuru::Config& videoSource_pt : pt["networked_video_sources"].as_array())
		{
			auto definitionPtr = std::make_shared<NetworkVideoSourceDefinition>();

			definitionPtr->readFromJSON(videoSource_pt);
			m_networkedVideoSourceList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}

	// Read in the spout video sources
	m_spoutVideoSourceList.clear();
	if (pt.has_key("spout_video_sources"))
	{
		for (const configuru::Config& videoSource_pt : pt["spout_video_sources"].as_array())
		{
			auto definitionPtr = std::make_shared<SpoutVideoSourceDefinition>();

			definitionPtr->readFromJSON(videoSource_pt);
			m_spoutVideoSourceList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

eVideoSourceType VideoSourceSystemConfig::getVideoSourceType(MikanVideoSourceID videoSourceId) const
{
	auto usbVideoSourcePtr = getUSBVideoSourceConfigConst(videoSourceId);
	if (usbVideoSourcePtr)
	{
		return eVideoSourceType::usb;
	}

	auto networkedVideoSourcePtr = getNetworkedVideoSourceConfigConst(videoSourceId);
	if (networkedVideoSourcePtr)
	{
		return eVideoSourceType::networked;
	}

	auto spoutVideoSourcePtr = getSpoutVideoSourceConfigConst(videoSourceId);
	if (spoutVideoSourcePtr)
	{
		return eVideoSourceType::spout;
	}

	return eVideoSourceType::INVALID;
}

bool VideoSourceSystemConfig::removeVideoSource(MikanVideoSourceID videoSourceId)
{
	switch (getVideoSourceType(videoSourceId))
	{
		case eVideoSourceType::usb:
			return removeUSBVideoSource(videoSourceId);
		case eVideoSourceType::networked:
			return removeNetworkedVideoSource(videoSourceId);
		case eVideoSourceType::spout:
			return removeSpoutVideoSource(videoSourceId);
		default:
			return false;
	}
}

USBVideoSourceDefinitionConstPtr VideoSourceSystemConfig::getUSBVideoSourceConfigConst(
	MikanVideoSourceID videoSourceId) const
{
	auto it = std::find_if(
		m_usbVideoSourceList.begin(), m_usbVideoSourceList.end(),
		[videoSourceId](USBVideoSourceDefinitionPtr configPtr) {
			return configPtr->getVideoSourceId() == videoSourceId;
		});

	if (it != m_usbVideoSourceList.end())
	{
		return USBVideoSourceDefinitionConstPtr(*it);
	}

	return USBVideoSourceDefinitionConstPtr();
}

USBVideoSourceDefinitionPtr VideoSourceSystemConfig::getUSBVideoSourceConfig(
	MikanVideoSourceID videoSourceId)
{
	return std::const_pointer_cast<USBVideoSourceDefinition>(
		getUSBVideoSourceConfigConst(videoSourceId));
}

MikanVideoSourceID VideoSourceSystemConfig::addUSBVideoSource(const std::string& videoSourceName)
{
	USBVideoSourceDefinitionPtr videoSourcePtr = 
		std::make_shared<USBVideoSourceDefinition>(m_nextVideoSourceId, videoSourceName);
	m_nextVideoSourceId++;

	m_usbVideoSourceList.push_back(videoSourcePtr);
	addChildConfig(videoSourcePtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_usbVideoSourceListPropertyId));

	return videoSourcePtr->getVideoSourceId();
}

bool VideoSourceSystemConfig::removeUSBVideoSource(MikanVideoSourceID videoSourceId)
{
	auto it = std::find_if(
		m_usbVideoSourceList.begin(), m_usbVideoSourceList.end(),
		[videoSourceId](USBVideoSourceDefinitionPtr configPtr) {
			return configPtr->getVideoSourceId() == videoSourceId;
		});

	if (it != m_usbVideoSourceList.end())
	{
		removeChildConfig(*it);

		m_usbVideoSourceList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_usbVideoSourceListPropertyId));

		return true;
	}

	return false;
}

NetworkVideoSourceDefinitionConstPtr VideoSourceSystemConfig::getNetworkedVideoSourceConfigConst(
	MikanVideoSourceID videoSourceId) const
{
	auto it = std::find_if(
		m_networkedVideoSourceList.begin(), m_networkedVideoSourceList.end(),
		[videoSourceId](NetworkVideoSourceDefinitionPtr configPtr) {
		return configPtr->getVideoSourceId() == videoSourceId;
	});

	if (it != m_networkedVideoSourceList.end())
	{
		return NetworkVideoSourceDefinitionConstPtr(*it);
	}

	return NetworkVideoSourceDefinitionConstPtr();
}

NetworkVideoSourceDefinitionPtr VideoSourceSystemConfig::getNetworkedVideoSourceConfig(
	MikanVideoSourceID videoSourceId)
{
	return std::const_pointer_cast<NetworkVideoSourceDefinition>(
		getNetworkedVideoSourceConfigConst(videoSourceId));
}

MikanVideoSourceID VideoSourceSystemConfig::addNetworkedVideoSource(const std::string& videoSourceName)
{
	NetworkVideoSourceDefinitionPtr videoSourcePtr =
		std::make_shared<NetworkVideoSourceDefinition>(m_nextVideoSourceId, videoSourceName);
	m_nextVideoSourceId++;

	m_networkedVideoSourceList.push_back(videoSourcePtr);
	addChildConfig(videoSourcePtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_networkedVideoSourceListPropertyId));

	return videoSourcePtr->getVideoSourceId();
}

bool VideoSourceSystemConfig::removeNetworkedVideoSource(MikanVideoSourceID videoSourceId)
{
	auto it = std::find_if(
		m_networkedVideoSourceList.begin(), m_networkedVideoSourceList.end(),
		[videoSourceId](NetworkVideoSourceDefinitionPtr configPtr) {
		return configPtr->getVideoSourceId() == videoSourceId;
	});

	if (it != m_networkedVideoSourceList.end())
	{
		removeChildConfig(*it);

		m_networkedVideoSourceList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_networkedVideoSourceListPropertyId));

		return true;
	}

	return false;
}

SpoutVideoSourceDefinitionConstPtr VideoSourceSystemConfig::getSpoutVideoSourceConfigConst(
	MikanVideoSourceID videoSourceId) const
{
	auto it = std::find_if(
		m_spoutVideoSourceList.begin(), m_spoutVideoSourceList.end(),
		[videoSourceId](SpoutVideoSourceDefinitionPtr configPtr) {
		return configPtr->getVideoSourceId() == videoSourceId;
	});

	if (it != m_spoutVideoSourceList.end())
	{
		return SpoutVideoSourceDefinitionConstPtr(*it);
	}

	return SpoutVideoSourceDefinitionConstPtr();
}

SpoutVideoSourceDefinitionPtr VideoSourceSystemConfig::getSpoutVideoSourceConfig(
	MikanVideoSourceID videoSourceId)
{
	return std::const_pointer_cast<SpoutVideoSourceDefinition>(
		getSpoutVideoSourceConfigConst(videoSourceId));
}

MikanVideoSourceID VideoSourceSystemConfig::addSpoutVideoSource(const std::string& videoSourceName)
{
	SpoutVideoSourceDefinitionPtr videoSourcePtr =
		std::make_shared<SpoutVideoSourceDefinition>(m_nextVideoSourceId, videoSourceName);
	m_nextVideoSourceId++;

	m_spoutVideoSourceList.push_back(videoSourcePtr);
	addChildConfig(videoSourcePtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_spoutVideoSourceListPropertyId));

	return videoSourcePtr->getVideoSourceId();
}

bool VideoSourceSystemConfig::removeSpoutVideoSource(MikanVideoSourceID videoSourceId)
{
	auto it = std::find_if(
		m_spoutVideoSourceList.begin(), m_spoutVideoSourceList.end(),
		[videoSourceId](SpoutVideoSourceDefinitionPtr configPtr) {
		return configPtr->getVideoSourceId() == videoSourceId;
	});

	if (it != m_spoutVideoSourceList.end())
	{
		removeChildConfig(*it);

		m_spoutVideoSourceList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_spoutVideoSourceListPropertyId));

		return true;
	}

	return false;
}
