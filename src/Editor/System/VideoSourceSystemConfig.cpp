#include "App.h"
#include "ClientVideoSourceComponent.h"
#include "NetworkVideoSourceComponent.h"
#include "SpoutVideoSourceComponent.h"
#include "USBVideoSourceComponent.h"
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

// -- VideoSourceSystemConfig -----
const std::string VideoSourceSystemConfig::k_clientVideoSourceListPropertyId= "clientVideoSourceList";
const std::string VideoSourceSystemConfig::k_usbVideoSourceListPropertyId= "usbVideoSourceList";
const std::string VideoSourceSystemConfig::k_networkedVideoSourceListPropertyId= "networkVideoSourceList";
const std::string VideoSourceSystemConfig::k_spoutVideoSourceListPropertyId= "spoutVideoSourceList";

configuru::Config VideoSourceSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["next_video_source_id"] = m_nextVideoSourceId;

	std::vector<configuru::Config> clientVideoSourceConfigs;
	for (auto videoSource : m_clientVideoSourceList)
	{
		clientVideoSourceConfigs.push_back(videoSource->writeToJSON());
	}
	pt.insert_or_assign(std::string("client_video_sources"), clientVideoSourceConfigs);

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

	std::vector<configuru::Config> usbVideoSourceConfigs;
	for (auto videoSource : m_usbVideoSourceList)
	{
		usbVideoSourceConfigs.push_back(videoSource->writeToJSON());
	}
	pt.insert_or_assign(std::string("usb_video_sources"), usbVideoSourceConfigs);

	return pt;
}

void VideoSourceSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextVideoSourceId = pt.get_or<int>("next_video_source_id", m_nextVideoSourceId);

	// Read in the client video sources
	m_clientVideoSourceList.clear();
	if (pt.has_key("client_video_sources"))
	{
		for (const configuru::Config& videoSource_pt : pt["client_video_sources"].as_array())
		{
			auto definitionPtr = std::make_shared<ClientVideoSourceDefinition>();

			definitionPtr->readFromJSON(videoSource_pt);
			m_clientVideoSourceList.push_back(definitionPtr);

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
}

eVideoSourceType VideoSourceSystemConfig::getVideoSourceType(MikanVideoSourceID videoSourceId) const
{
	auto clientVideoSourcePtr = getClientVideoSourceConfigConst(videoSourceId);
	if (clientVideoSourcePtr)
	{
		return eVideoSourceType::client;
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

	auto usbVideoSourcePtr = getUSBVideoSourceConfigConst(videoSourceId);
	if (usbVideoSourcePtr)
	{
		return eVideoSourceType::usb;
	}

	return eVideoSourceType::INVALID;
}

bool VideoSourceSystemConfig::removeVideoSource(MikanVideoSourceID videoSourceId)
{
	switch (getVideoSourceType(videoSourceId))
	{
		case eVideoSourceType::client:
			return removeClientVideoSource(videoSourceId);
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

ClientVideoSourceDefinitionConstPtr VideoSourceSystemConfig::getClientVideoSourceConfigConst(
	MikanVideoSourceID videoSourceId) const
{
	auto it = std::find_if(
		m_clientVideoSourceList.begin(), m_clientVideoSourceList.end(),
		[videoSourceId](ClientVideoSourceDefinitionPtr configPtr) {
			return configPtr->getVideoSourceId() == videoSourceId;
		});
	if (it != m_clientVideoSourceList.end())
	{
		return ClientVideoSourceDefinitionConstPtr(*it);
	}
	return ClientVideoSourceDefinitionConstPtr();
}

ClientVideoSourceDefinitionPtr VideoSourceSystemConfig::getClientVideoSourceConfig(
	MikanVideoSourceID videoSourceId)
{
	auto constPtr = getClientVideoSourceConfigConst(videoSourceId);
	if (constPtr)
	{
		return std::const_pointer_cast<ClientVideoSourceDefinition>(constPtr);
	}
	return ClientVideoSourceDefinitionPtr();
}

MikanVideoSourceID VideoSourceSystemConfig::addClientVideoSource(
	const MikanClientVideoSourceInfo& videoSourceInfo)
{
	ClientVideoSourceDefinitionPtr videoSourcePtr =
		std::make_shared<ClientVideoSourceDefinition>(m_nextVideoSourceId, videoSourceInfo);
	m_nextVideoSourceId++;

	m_clientVideoSourceList.push_back(videoSourcePtr);
	addChildConfig(videoSourcePtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_clientVideoSourceListPropertyId));

	return videoSourcePtr->getVideoSourceId();
}

bool VideoSourceSystemConfig::removeClientVideoSource(
	MikanVideoSourceID videoSourceId)
{
	auto it = std::find_if(
		m_clientVideoSourceList.begin(), m_clientVideoSourceList.end(),
		[videoSourceId](ClientVideoSourceDefinitionPtr configPtr) {
			return configPtr->getVideoSourceId() == videoSourceId;
		});

	if (it != m_clientVideoSourceList.end())
	{
		removeChildConfig(*it);

		m_clientVideoSourceList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_clientVideoSourceListPropertyId));

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

MikanVideoSourceID VideoSourceSystemConfig::addNetworkedVideoSource(
	const MikanNetworkVideoSourceInfo& videoSourceInfo)
{
	NetworkVideoSourceDefinitionPtr videoSourcePtr =
		std::make_shared<NetworkVideoSourceDefinition>(m_nextVideoSourceId, videoSourceInfo);
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

MikanVideoSourceID VideoSourceSystemConfig::addSpoutVideoSource(
	const MikanSpoutVideoSourceInfo& videoSourceInfo)
{
	SpoutVideoSourceDefinitionPtr videoSourcePtr =
		std::make_shared<SpoutVideoSourceDefinition>(m_nextVideoSourceId, videoSourceInfo);
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

MikanVideoSourceID VideoSourceSystemConfig::addUSBVideoSource(
	const MikanUSBVideoSourceInfo& videoSourceInfo)
{
	USBVideoSourceDefinitionPtr videoSourcePtr =
		std::make_shared<USBVideoSourceDefinition>(m_nextVideoSourceId, videoSourceInfo);
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
