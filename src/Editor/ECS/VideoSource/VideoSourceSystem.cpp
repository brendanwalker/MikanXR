#include "VideoSourceSystem.h"
#include "CameraComponent.h"
#include "ClientVideoSourceSystem.h"
#include "ClientVideoSourceComponent.h"
#include "CameraObjectSystem.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "MikanObject.h"
#include "NetworkVideoSourceSystem.h"
#include "NetworkVideoSourceComponent.h"
#include "ProjectManager.h"
#include "SceneObjectSystem.h"
#include "SceneComponent.h"
#include "SpoutVideoSourceSystem.h"
#include "SpoutVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "USBVideoSourceComponent.h"
#include "App.h"
#include "ProjectConfig.h"

VideoSourceSystemWeakPtr VideoSourceSystem::s_VideoSourceSystem;

bool VideoSourceSystem::init()
{
	MikanObjectSystem::init();

	// Create subsystems
	auto* owner= getOwnerObjectSystemManager();
	m_clientVideoSourceSystem = owner->getSystemOfType<ClientVideoSourceSystem>();
	m_networkVideoSourceSystem = owner->getSystemOfType<NetworkVideoSourceSystem>();
	m_spoutVideoSourceSystem = owner->getSystemOfType<SpoutVideoSourceSystem>();
	m_usbVideoSourceSystem = owner->getSystemOfType<USBVideoSourceSystem>();

	s_VideoSourceSystem = std::static_pointer_cast<VideoSourceSystem>(shared_from_this());
	return true;
}

void VideoSourceSystem::dispose()
{
	s_VideoSourceSystem.reset();

	m_clientVideoSourceSystem.reset();
	m_networkVideoSourceSystem.reset();
	m_spoutVideoSourceSystem.reset();
	m_usbVideoSourceSystem.reset();

	MikanObjectSystem::dispose();
}

void VideoSourceSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	auto clientVideoSource = objectPtr->getComponentOfType<ClientVideoSourceComponent>();
	if (clientVideoSource != nullptr)
	{
		m_clientVideoSourceSystem->removeClientVideoSource(
			clientVideoSource->getVideoSourceDefinition()->getVideoSourceId());
		return;
	}

	auto networkVideoSource = objectPtr->getComponentOfType<NetworkVideoSourceComponent>();
	if (networkVideoSource != nullptr)
	{
		m_networkVideoSourceSystem->removeNetworkVideoSource(
			networkVideoSource->getVideoSourceDefinition()->getVideoSourceId());
		return;
	}

	auto spoutVideoSource = objectPtr->getComponentOfType<SpoutVideoSourceComponent>();
	if (spoutVideoSource != nullptr)
	{
		m_spoutVideoSourceSystem->removeSpoutVideoSource(
			spoutVideoSource->getVideoSourceDefinition()->getVideoSourceId());
		return;
	}

	auto usbVideoSource = objectPtr->getComponentOfType<USBVideoSourceComponent>();
	if (usbVideoSource != nullptr)
	{
		m_usbVideoSourceSystem->removeUSBVideoSource(
			usbVideoSource->getVideoSourceDefinition()->getVideoSourceId());
		return;
	}
}

VideoSourceSystemConfigConstPtr VideoSourceSystem::getVideoSourceSystemConfigConst() const
{
	return getProjectConfig()->videoSourceSystemConfig;
}

VideoSourceSystemConfigPtr VideoSourceSystem::getVideoSourceSystemConfig()
{
	return std::const_pointer_cast<VideoSourceSystemConfig>(getVideoSourceSystemConfigConst());
}

VideoSourceIdList VideoSourceSystem::getVideoSourceIdList() const
{
	VideoSourceIdList videoSourceIdList;
	
	auto clientVideoSourceIds = m_clientVideoSourceSystem->getVideoSourceIdList();
	videoSourceIdList.insert(videoSourceIdList.end(), clientVideoSourceIds.begin(), clientVideoSourceIds.end());
	auto networkVideoSourceIds = m_networkVideoSourceSystem->getVideoSourceIdList();
	videoSourceIdList.insert(videoSourceIdList.end(), networkVideoSourceIds.begin(), networkVideoSourceIds.end());
	auto spoutVideoSourceIds = m_spoutVideoSourceSystem->getVideoSourceIdList();
	videoSourceIdList.insert(videoSourceIdList.end(), spoutVideoSourceIds.begin(), spoutVideoSourceIds.end());
	auto usbVideoSourceIds = m_usbVideoSourceSystem->getVideoSourceIdList();
	videoSourceIdList.insert(videoSourceIdList.end(), usbVideoSourceIds.begin(), usbVideoSourceIds.end());

	return videoSourceIdList;
}

VideoSourceComponentPtr VideoSourceSystem::getVideoSourceById(MikanVideoSourceID VideoSourceId) const
{
	auto clientVideoSourcePtr = m_clientVideoSourceSystem->getClientVideoSourceById(VideoSourceId);
	if (clientVideoSourcePtr)
	{
		return clientVideoSourcePtr;
	}

	auto networkVideoSourcePtr = m_networkVideoSourceSystem->getNetworkVideoSourceById(VideoSourceId);
	if (networkVideoSourcePtr)
	{
		return networkVideoSourcePtr;
	}

	auto spoutVideoSourcePtr = m_spoutVideoSourceSystem->getSpoutVideoSourceById(VideoSourceId);
	if (spoutVideoSourcePtr)
	{
		return spoutVideoSourcePtr;
	}

	auto usbVideoSourcePtr = m_usbVideoSourceSystem->getUSBVideoSourceById(VideoSourceId);
	if (usbVideoSourcePtr)
	{
		return usbVideoSourcePtr;
	}

	return VideoSourceComponentPtr();
}

eVideoSourceType VideoSourceSystem::getVideoSourceType(MikanVideoSourceID VideoSourceId) const
{
	auto clientVideoSourcePtr = m_clientVideoSourceSystem->getClientVideoSourceById(VideoSourceId);
	if (clientVideoSourcePtr)
	{
		return eVideoSourceType::client;
	}

	auto networkVideoSourcePtr = m_networkVideoSourceSystem->getNetworkVideoSourceById(VideoSourceId);
	if (networkVideoSourcePtr)
	{
		return eVideoSourceType::networked;
	}

	auto spoutVideoSourcePtr = m_spoutVideoSourceSystem->getSpoutVideoSourceById(VideoSourceId);
	if (spoutVideoSourcePtr)
	{
		return eVideoSourceType::spout;
	}

	auto usbVideoSourcePtr = m_usbVideoSourceSystem->getUSBVideoSourceById(VideoSourceId);
	if (usbVideoSourcePtr)
	{
		return eVideoSourceType::usb;
	}

	return eVideoSourceType::INVALID;
}

bool VideoSourceSystem::removeVideoSource(MikanVideoSourceID videoSourceId)
{
	switch (getVideoSourceType(videoSourceId))
	{
		case eVideoSourceType::client:
			return m_clientVideoSourceSystem->removeClientVideoSource(videoSourceId);
			break;
		case eVideoSourceType::networked:
			return m_networkVideoSourceSystem->removeNetworkVideoSource(videoSourceId);
			break;
		case eVideoSourceType::spout:
			return m_spoutVideoSourceSystem->removeSpoutVideoSource(videoSourceId);
			break;
		case eVideoSourceType::usb:
			return m_usbVideoSourceSystem->removeUSBVideoSource(videoSourceId);
			break;
	}

	return false;
}