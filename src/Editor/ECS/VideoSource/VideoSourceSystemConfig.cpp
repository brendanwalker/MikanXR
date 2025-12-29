#include "App.h"
#include "NetworkVideoSourceComponent.h"
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
const std::string VideoSourceSystemConfig::k_usbVideoSourceListPropertyId= "usbVideoSourceList";
const std::string VideoSourceSystemConfig::k_networkedVideoSourceListPropertyId= "networkVideoSourceList";

configuru::Config VideoSourceSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["next_video_source_id"] = m_nextVideoSourceId;

	std::vector<configuru::Config> networkedVideoSourceConfigs;
	for (auto videoSource : m_networkedVideoSourceList)
	{
		networkedVideoSourceConfigs.push_back(videoSource->writeToJSON());
	}
	pt.insert_or_assign(std::string("networked_video_sources"), networkedVideoSourceConfigs);

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
	auto networkedVideoSourcePtr = getNetworkedVideoSourceConfigConst(videoSourceId);
	if (networkedVideoSourcePtr)
	{
		return eVideoSourceType::networked;
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
		case eVideoSourceType::usb:
			return removeUSBVideoSourceDefinition(videoSourceId);
		case eVideoSourceType::networked:
			return removeNetworkedVideoSourceDefinition(videoSourceId);
		default:
			return false;
	}
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

NetworkVideoSourceDefinitionPtr VideoSourceSystemConfig::allocateNetworkedVideoSourceDefinition(
	const MikanNetworkVideoSourceInfo& videoSourceInfo)
{
	NetworkVideoSourceDefinitionPtr videoSourcePtr =
		std::make_shared<NetworkVideoSourceDefinition>(m_nextVideoSourceId, videoSourceInfo);
	m_nextVideoSourceId++;

	return videoSourcePtr;
}

bool VideoSourceSystemConfig::addNetworkedVideoSourceDefinition(
	NetworkVideoSourceDefinitionPtr videoSourceDefinitionPtr)
{
	if (!getNetworkedVideoSourceConfig(videoSourceDefinitionPtr->getVideoSourceId()))
	{
		m_networkedVideoSourceList.push_back(videoSourceDefinitionPtr);
		addChildConfig(videoSourceDefinitionPtr);

		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_networkedVideoSourceListPropertyId));
		return true;
	}

	return false;
}

bool VideoSourceSystemConfig::removeNetworkedVideoSourceDefinition(MikanVideoSourceID videoSourceId)
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
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_networkedVideoSourceListPropertyId));

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

USBVideoSourceDefinitionPtr VideoSourceSystemConfig::allocateUSBVideoSourceDefinition(
	const MikanUSBVideoSourceInfo& videoSourceInfo)
{
	USBVideoSourceDefinitionPtr videoSourcePtr =
		std::make_shared<USBVideoSourceDefinition>(m_nextVideoSourceId, videoSourceInfo);
	m_nextVideoSourceId++;

	return videoSourcePtr;
}

bool VideoSourceSystemConfig::addUSBVideoSourceDefinition(
	USBVideoSourceDefinitionPtr videoSourceDefinitionPtr)
{
	if (!getUSBVideoSourceConfig(videoSourceDefinitionPtr->getVideoSourceId()))
	{
		m_usbVideoSourceList.push_back(videoSourceDefinitionPtr);
		addChildConfig(videoSourceDefinitionPtr);

		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_usbVideoSourceListPropertyId));
		return true;
	}

	return false;
}

bool VideoSourceSystemConfig::removeUSBVideoSourceDefinition(MikanVideoSourceID videoSourceId)
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
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_usbVideoSourceListPropertyId));

		return true;
	}

	return false;
}
