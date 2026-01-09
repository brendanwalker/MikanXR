#include "VideoSourceSystem.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "MikanObject.h"
#include "NetworkVideoSourceSystem.h"
#include "NetworkVideoSourceComponent.h"
#include "ProjectManager.h"
#include "SceneObjectSystem.h"
#include "SceneComponent.h"
#include "USBVideoSourceSystem.h"
#include "USBVideoSourceComponent.h"
#include "App.h"
#include "ProjectConfig.h"

VideoSourceSystemWeakPtr VideoSourceSystem::s_VideoSourceSystem;

bool VideoSourceSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

	// Create subsystems
	auto* owner= getOwnerProjectManager();
	m_networkVideoSourceSystem = owner->getSystemOfType<NetworkVideoSourceSystem>();
	m_usbVideoSourceSystem = owner->getSystemOfType<USBVideoSourceSystem>();

	s_VideoSourceSystem = std::static_pointer_cast<VideoSourceSystem>(shared_from_this());
	return true;
}

void VideoSourceSystem::dispose()
{
	s_VideoSourceSystem.reset();

	m_networkVideoSourceSystem.reset();
	m_usbVideoSourceSystem.reset();

	MikanObjectSystem::dispose();
}

void VideoSourceSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	auto networkVideoSource = objectPtr->getComponentOfType<NetworkVideoSourceComponent>();
	if (networkVideoSource != nullptr)
	{
		m_networkVideoSourceSystem->removeNetworkVideoSource(
			networkVideoSource->getVideoSourceDefinition()->getVideoSourceId());
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

MikanComponentPtr VideoSourceSystem::getComponentById(int componentId) const
{
	return getVideoSourceById(componentId);
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
	
	auto networkVideoSourceIds = m_networkVideoSourceSystem->getVideoSourceIdList();
	videoSourceIdList.insert(videoSourceIdList.end(), networkVideoSourceIds.begin(), networkVideoSourceIds.end());
	auto usbVideoSourceIds = m_usbVideoSourceSystem->getVideoSourceIdList();
	videoSourceIdList.insert(videoSourceIdList.end(), usbVideoSourceIds.begin(), usbVideoSourceIds.end());

	return videoSourceIdList;
}

VideoSourceComponentPtr VideoSourceSystem::getVideoSourceById(MikanVideoSourceID VideoSourceId) const
{
	auto networkVideoSourcePtr = m_networkVideoSourceSystem->getNetworkVideoSourceById(VideoSourceId);
	if (networkVideoSourcePtr)
	{
		return networkVideoSourcePtr;
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
	auto networkVideoSourcePtr = m_networkVideoSourceSystem->getNetworkVideoSourceById(VideoSourceId);
	if (networkVideoSourcePtr)
	{
		return eVideoSourceType::networked;
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
		case eVideoSourceType::networked:
			return m_networkVideoSourceSystem->removeNetworkVideoSource(videoSourceId);
			break;
		case eVideoSourceType::usb:
			return m_usbVideoSourceSystem->removeUSBVideoSource(videoSourceId);
			break;
	}

	return false;
}