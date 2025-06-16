#include "App.h"
#include "ClientVideoSourceComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "NetworkVideoSourceComponent.h"
#include "SpoutVideoSourceComponent.h"
#include "ProjectConfig.h"
#include "USBVideoSourceComponent.h"
#include "VideoSourceSystem.h"

VideoSourceSystemWeakPtr VideoSourceSystem::s_VideoSourceSystem;

bool VideoSourceSystem::init()
{
	MikanObjectSystem::init();

	VideoSourceSystemConfigConstPtr videoSourceSystemConfig = getVideoSourceSystemConfigConst();

	for (const auto& sourceConfig : videoSourceSystemConfig->getClientVideoSourceList())
	{
		createClientVideoSourceObject(sourceConfig);
	}

	for (const auto& sourceConfig : videoSourceSystemConfig->getNetworkedVideoSourceList())
	{
		createNetworkVideoSourceObject(sourceConfig);
	}

	for (const auto& sourceConfig : videoSourceSystemConfig->getSpoutVideoSourceList())
	{
		createSpoutVideoSourceObject(sourceConfig);
	}

	for (const auto& sourceConfig : videoSourceSystemConfig->getUSBVideoSourceList())
	{
		createUSBVideoSourceObject(sourceConfig);
	}

	s_VideoSourceSystem = std::static_pointer_cast<VideoSourceSystem>(shared_from_this());
	return true;
}

void VideoSourceSystem::dispose()
{
	s_VideoSourceSystem.reset();

	m_clientVideoSourceComponents.clear();
	m_networkVideoSourceComponents.clear();
	m_spoutVideoSourceComponents.clear();
	m_usbVideoSourceComponents.clear();

	MikanObjectSystem::dispose();
}

void VideoSourceSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	auto clientVideoSource = objectPtr->getComponentOfType<ClientVideoSourceComponent>();
	if (clientVideoSource != nullptr)
	{
		removeClientVideoSource(clientVideoSource->getVideoSourceDefinition()->getVideoSourceId());
		return;
	}

	auto networkVideoSource = objectPtr->getComponentOfType<NetworkVideoSourceComponent>();
	if (networkVideoSource != nullptr)
	{
		removeClientVideoSource(networkVideoSource->getVideoSourceDefinition()->getVideoSourceId());
		return;
	}

	auto spoutVideoSource = objectPtr->getComponentOfType<SpoutVideoSourceComponent>();
	if (spoutVideoSource != nullptr)
	{
		removeClientVideoSource(spoutVideoSource->getVideoSourceDefinition()->getVideoSourceId());
		return;
	}

	auto usbVideoSource = objectPtr->getComponentOfType<USBVideoSourceComponent>();
	if (usbVideoSource != nullptr)
	{
		removeClientVideoSource(usbVideoSource->getVideoSourceDefinition()->getVideoSourceId());
		return;
	}
}

VideoSourceSystemConfigConstPtr VideoSourceSystem::getVideoSourceSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->videoSourceSystemConfig;
}

VideoSourceSystemConfigPtr VideoSourceSystem::getVideoSourceSystemConfig()
{
	return std::const_pointer_cast<VideoSourceSystemConfig>(getVideoSourceSystemConfigConst());
}

VideoSourceComponentPtr VideoSourceSystem::getVideoSourceById(MikanVideoSourceID VideoSourceId) const
{
	auto clientVideoSourcePtr = getClientVideoSourceById(VideoSourceId);
	if (clientVideoSourcePtr)
	{
		return clientVideoSourcePtr;
	}

	auto networkVideoSourcePtr = getNetworkVideoSourceById(VideoSourceId);
	if (networkVideoSourcePtr)
	{
		return networkVideoSourcePtr;
	}

	auto spoutVideoSourcePtr = getSpoutVideoSourceById(VideoSourceId);
	if (spoutVideoSourcePtr)
	{
		return spoutVideoSourcePtr;
	}

	auto usbVideoSourcePtr = getUSBVideoSourceById(VideoSourceId);
	if (usbVideoSourcePtr)
	{
		return usbVideoSourcePtr;
	}

	return VideoSourceComponentPtr();
}

eVideoSourceType VideoSourceSystem::getVideoSourceType(MikanVideoSourceID VideoSourceId) const
{
	auto clientVideoSourcePtr = getClientVideoSourceById(VideoSourceId);
	if (clientVideoSourcePtr)
	{
		return eVideoSourceType::client;
	}

	auto networkVideoSourcePtr = getNetworkVideoSourceById(VideoSourceId);
	if (networkVideoSourcePtr)
	{
		return eVideoSourceType::networked;
	}

	auto spoutVideoSourcePtr = getSpoutVideoSourceById(VideoSourceId);
	if (spoutVideoSourcePtr)
	{
		return eVideoSourceType::spout;
	}

	auto usbVideoSourcePtr = getUSBVideoSourceById(VideoSourceId);
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
			return removeClientVideoSource(videoSourceId);
			break;
		case eVideoSourceType::networked:
			return removeNetworkVideoSource(videoSourceId);
			break;
		case eVideoSourceType::spout:
			return removeSpoutVideoSource(videoSourceId);
			break;
		case eVideoSourceType::usb:
			return removeUSBVideoSource(videoSourceId);
			break;
	}

	return false;
}

// -- Client Video Sources -----
ClientVideoSourceComponentPtr VideoSourceSystem::getClientVideoSourceById(MikanVideoSourceID videoSourceId) const
{
	auto iter = m_clientVideoSourceComponents.find(videoSourceId);
	if (iter != m_clientVideoSourceComponents.end())
	{
		return iter->second.lock();
	}

	return ClientVideoSourceComponentPtr();
}

ClientVideoSourceComponentPtr VideoSourceSystem::getClientVideoSourceByName(const std::string& videoSourceName) const
{
	for (auto it = m_clientVideoSourceComponents.begin(); it != m_clientVideoSourceComponents.end(); it++)
	{
		ClientVideoSourceComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == videoSourceName)
		{
			return componentPtr;
		}
	}

	return ClientVideoSourceComponentPtr();
}

ClientVideoSourceComponentPtr VideoSourceSystem::addNewClientVideoSource(
	const MikanClientVideoSourceInfo& videoSourceInfo)
{
	VideoSourceSystemConfigPtr videoSourceSystemConfig = getVideoSourceSystemConfig();

	MikanVideoSourceID videoSourceId = videoSourceSystemConfig->addClientVideoSource(videoSourceInfo);
	if (videoSourceId != INVALID_MIKAN_ID)
	{
		ClientVideoSourceDefinitionPtr configPtr = videoSourceSystemConfig->getClientVideoSourceConfig(videoSourceId);
		assert(configPtr != nullptr);

		return createClientVideoSourceObject(configPtr);
	}

	return ClientVideoSourceComponentPtr();
}

bool VideoSourceSystem::removeClientVideoSource(MikanVideoSourceID videoSourceId)
{
	return
		disposeClientVideoSourceObject(videoSourceId) &&
		getVideoSourceSystemConfig()->removeVideoSource(videoSourceId);
}

ClientVideoSourceComponentPtr VideoSourceSystem::createClientVideoSourceObject(
	ClientVideoSourceDefinitionPtr videoSourceDefinition)
{
	MikanObjectPtr videoSourceObject = newObject();
	videoSourceObject->setName(videoSourceDefinition->getComponentName());

	// Make the ClientVideoSource component the root of the object
	auto videoSourceComponentPtr = videoSourceObject->addComponent<ClientVideoSourceComponent>();
	videoSourceComponentPtr->setDefinition(videoSourceDefinition);

	// Init the object once all components are added
	videoSourceObject->init();

	// Keep track of all the client video sources in the system
	m_clientVideoSourceComponents.insert({videoSourceDefinition->getVideoSourceId(), videoSourceComponentPtr});

	return videoSourceComponentPtr;
}

bool VideoSourceSystem::disposeClientVideoSourceObject(MikanVideoSourceID videoSourceId)
{
	auto it = m_clientVideoSourceComponents.find(videoSourceId);
	if (it != m_clientVideoSourceComponents.end())
	{
		ClientVideoSourceComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_clientVideoSourceComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}

// -- Network Video Sources -----
NetworkVideoSourceComponentPtr VideoSourceSystem::getNetworkVideoSourceById(MikanVideoSourceID videoSourceId) const
{
	auto iter = m_networkVideoSourceComponents.find(videoSourceId);
	if (iter != m_networkVideoSourceComponents.end())
	{
		return iter->second.lock();
	}

	return NetworkVideoSourceComponentPtr();
}

NetworkVideoSourceComponentPtr VideoSourceSystem::getNetworkVideoSourceByName(const std::string& videoSourceName) const
{
	for (auto it = m_networkVideoSourceComponents.begin(); it != m_networkVideoSourceComponents.end(); it++)
	{
		NetworkVideoSourceComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == videoSourceName)
		{
			return componentPtr;
		}
	}

	return NetworkVideoSourceComponentPtr();
}

NetworkVideoSourceComponentPtr VideoSourceSystem::addNewNetworkVideoSource(
	const MikanNetworkVideoSourceInfo& videoSourceInfo)
{
	VideoSourceSystemConfigPtr videoSourceSystemConfig = getVideoSourceSystemConfig();

	MikanVideoSourceID videoSourceId = videoSourceSystemConfig->addNetworkedVideoSource(videoSourceInfo);
	if (videoSourceId != INVALID_MIKAN_ID)
	{
		NetworkVideoSourceDefinitionPtr configPtr = videoSourceSystemConfig->getNetworkedVideoSourceConfig(videoSourceId);
		assert(configPtr != nullptr);

		return createNetworkVideoSourceObject(configPtr);
	}

	return NetworkVideoSourceComponentPtr();
}

bool VideoSourceSystem::removeNetworkVideoSource(MikanVideoSourceID videoSourceId)
{
	return
		disposeNetworkVideoSourceObject(videoSourceId) &&
		getVideoSourceSystemConfig()->removeVideoSource(videoSourceId);
}

NetworkVideoSourceComponentPtr VideoSourceSystem::createNetworkVideoSourceObject(
	NetworkVideoSourceDefinitionPtr videoSourceDefinition)
{
	MikanObjectPtr videoSourceObject = newObject();
	videoSourceObject->setName(videoSourceDefinition->getComponentName());

	// Make the ClientVideoSource component the root of the object
	auto videoSourceComponentPtr = videoSourceObject->addComponent<NetworkVideoSourceComponent>();
	videoSourceComponentPtr->setDefinition(videoSourceDefinition);

	// Init the object once all components are added
	videoSourceObject->init();

	// Keep track of all the network video sources in the system
	m_networkVideoSourceComponents.insert({ videoSourceDefinition->getVideoSourceId(), videoSourceComponentPtr });

	return videoSourceComponentPtr;
}

bool VideoSourceSystem::disposeNetworkVideoSourceObject(MikanVideoSourceID videoSourceId)
{
	auto it = m_networkVideoSourceComponents.find(videoSourceId);
	if (it != m_networkVideoSourceComponents.end())
	{
		NetworkVideoSourceComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_networkVideoSourceComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}

// -- Spout Video Sources -----
SpoutVideoSourceComponentPtr VideoSourceSystem::getSpoutVideoSourceById(MikanVideoSourceID videoSourceId) const
{
	auto iter = m_spoutVideoSourceComponents.find(videoSourceId);
	if (iter != m_spoutVideoSourceComponents.end())
	{
		return iter->second.lock();
	}

	return SpoutVideoSourceComponentPtr();
}

SpoutVideoSourceComponentPtr VideoSourceSystem::getSpoutVideoSourceByName(const std::string& videoSourceName) const
{
	for (auto it = m_spoutVideoSourceComponents.begin(); it != m_spoutVideoSourceComponents.end(); it++)
	{
		SpoutVideoSourceComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == videoSourceName)
		{
			return componentPtr;
		}
	}

	return SpoutVideoSourceComponentPtr();
}

SpoutVideoSourceComponentPtr VideoSourceSystem::addNewSpoutVideoSource(
	const MikanSpoutVideoSourceInfo& videoSourceInfo)
{
	VideoSourceSystemConfigPtr videoSourceSystemConfig = getVideoSourceSystemConfig();

	MikanVideoSourceID videoSourceId = videoSourceSystemConfig->addSpoutVideoSource(videoSourceInfo);
	if (videoSourceId != INVALID_MIKAN_ID)
	{
		SpoutVideoSourceDefinitionPtr configPtr = videoSourceSystemConfig->getSpoutVideoSourceConfig(videoSourceId);
		assert(configPtr != nullptr);

		return createSpoutVideoSourceObject(configPtr);
	}

	return SpoutVideoSourceComponentPtr();
}

bool VideoSourceSystem::removeSpoutVideoSource(MikanVideoSourceID videoSourceId)
{
	return
		disposeSpoutVideoSourceObject(videoSourceId) &&
		getVideoSourceSystemConfig()->removeVideoSource(videoSourceId);
}

SpoutVideoSourceComponentPtr VideoSourceSystem::createSpoutVideoSourceObject(
	SpoutVideoSourceDefinitionPtr videoSourceDefinition)
{
	MikanObjectPtr videoSourceObject = newObject();
	videoSourceObject->setName(videoSourceDefinition->getComponentName());

	// Make the ClientVideoSource component the root of the object
	auto videoSourceComponentPtr = videoSourceObject->addComponent<SpoutVideoSourceComponent>();
	videoSourceComponentPtr->setDefinition(videoSourceDefinition);

	// Init the object once all components are added
	videoSourceObject->init();

	// Keep track of all the spout video sources in the system
	m_spoutVideoSourceComponents.insert({ videoSourceDefinition->getVideoSourceId(), videoSourceComponentPtr });

	return videoSourceComponentPtr;
}

bool VideoSourceSystem::disposeSpoutVideoSourceObject(MikanVideoSourceID videoSourceId)
{
	auto it = m_spoutVideoSourceComponents.find(videoSourceId);
	if (it != m_spoutVideoSourceComponents.end())
	{
		SpoutVideoSourceComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_spoutVideoSourceComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}

// -- USB Video Sources -----
USBVideoSourceComponentPtr VideoSourceSystem::getUSBVideoSourceById(MikanVideoSourceID videoSourceId) const
{
	auto iter = m_usbVideoSourceComponents.find(videoSourceId);
	if (iter != m_usbVideoSourceComponents.end())
	{
		return iter->second.lock();
	}

	return USBVideoSourceComponentPtr();
}

USBVideoSourceComponentPtr VideoSourceSystem::getUSBVideoSourceByName(const std::string& videoSourceName) const
{
	for (auto it = m_usbVideoSourceComponents.begin(); it != m_usbVideoSourceComponents.end(); it++)
	{
		USBVideoSourceComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == videoSourceName)
		{
			return componentPtr;
		}
	}

	return USBVideoSourceComponentPtr();
}

USBVideoSourceComponentPtr VideoSourceSystem::addNewUSBVideoSource(
	const MikanUSBVideoSourceInfo& videoSourceInfo)
{
	VideoSourceSystemConfigPtr videoSourceSystemConfig = getVideoSourceSystemConfig();

	MikanVideoSourceID videoSourceId = videoSourceSystemConfig->addUSBVideoSource(videoSourceInfo);
	if (videoSourceId != INVALID_MIKAN_ID)
	{
		USBVideoSourceDefinitionPtr configPtr = videoSourceSystemConfig->getUSBVideoSourceConfig(videoSourceId);
		assert(configPtr != nullptr);

		return createUSBVideoSourceObject(configPtr);
	}

	return USBVideoSourceComponentPtr();
}

bool VideoSourceSystem::removeUSBVideoSource(MikanVideoSourceID videoSourceId)
{
	return
		disposeUSBVideoSourceObject(videoSourceId) &&
		getVideoSourceSystemConfig()->removeVideoSource(videoSourceId);
}

USBVideoSourceComponentPtr VideoSourceSystem::createUSBVideoSourceObject(
	USBVideoSourceDefinitionPtr videoSourceDefinition)
{
	MikanObjectPtr videoSourceObject = newObject();
	videoSourceObject->setName(videoSourceDefinition->getComponentName());

	// Make the ClientVideoSource component the root of the object
	auto videoSourceComponentPtr = videoSourceObject->addComponent<USBVideoSourceComponent>();
	videoSourceComponentPtr->setDefinition(videoSourceDefinition);

	// Init the object once all components are added
	videoSourceObject->init();

	// Keep track of all the usb video sources in the system
	m_usbVideoSourceComponents.insert({ videoSourceDefinition->getVideoSourceId(), videoSourceComponentPtr });

	return videoSourceComponentPtr;
}

bool VideoSourceSystem::disposeUSBVideoSourceObject(MikanVideoSourceID videoSourceId)
{
	auto it = m_usbVideoSourceComponents.find(videoSourceId);
	if (it != m_usbVideoSourceComponents.end())
	{
		USBVideoSourceComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_usbVideoSourceComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}