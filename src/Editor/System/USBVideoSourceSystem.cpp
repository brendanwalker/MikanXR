#include "USBVideoSourceSystem.h"
#include "VideoSourceSystem.h"
#include "USBVideoSourceComponent.h"
#include "IUsbVideoDeviceModule.h"
#include "Logger.h"
#include "MikanModuleManager.h"
#include "App.h"
#include "ProjectConfig.h"
#include "VideoSourceSystemConfig.h"
#include "MikanObject.h"

#include <assert.h>

bool USBVideoSourceSystem::init()
{
	MikanObjectSystem::init();

    VideoSourceSystemConfigConstPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    for (const auto& sourceConfig : videoSourceSystemConfig->getUSBVideoSourceList())
    {
        createUSBVideoSourceObject(sourceConfig);
    }

    return true;
}

void USBVideoSourceSystem::dispose()
{
    m_usbVideoSourceComponents.clear();
    disposeUsbVideoDeviceManager();
	MikanObjectSystem::dispose();
}

bool USBVideoSourceSystem::createUsbVideoDeviceManager(const std::string& moduleName)
{
    // Bail if we didn't select a valid runtime type to use
    if (moduleName.empty())
        return false;

    // Attempt to load the vr device module
    m_usbVideoDeviceModule = getMikanModuleManager()->getModule<IUsbVideoDeviceModule>(moduleName);
    if (!m_usbVideoDeviceModule)
    {
        MIKAN_LOG_ERROR("USBVideoSourceSystem::createUsbVideoDeviceManager") << "Failed to load module" << moduleName;
        return false;
    }

    // Attempt to create a vr device manager
    m_usbVideoDeviceManager = m_usbVideoDeviceModule->createUsbVideoDeviceManager();
    if (!m_usbVideoDeviceManager)
    {
        MIKAN_LOG_WARNING("USBVideoSourceSystem::createUsbVideoDeviceManager") << "Failed to create UsbVideoDeviceManager";
        return false;
    }

    // Listen for device manager changes
    //m_usbVideoDeviceManager->addListener(this);

    return true;
}

void USBVideoSourceSystem::disposeUsbVideoDeviceManager()
{
    if (m_usbVideoDeviceManager)
    {
        //m_usbVideoDeviceManager->removeListener(this);
        m_usbVideoDeviceManager->shutdown();
        m_usbVideoDeviceManager = nullptr;
    }

    if (m_usbVideoDeviceModule)
    {
        getMikanModuleManager()->disposeModule(m_usbVideoDeviceModule);
        m_usbVideoDeviceModule = nullptr;
    }
}

USBVideoSourceComponentPtr USBVideoSourceSystem::getUSBVideoSourceById(MikanVideoSourceID videoSourceId) const
{
    auto iter = m_usbVideoSourceComponents.find(videoSourceId);
    if (iter != m_usbVideoSourceComponents.end())
    {
        return iter->second.lock();
    }

    return USBVideoSourceComponentPtr();
}

USBVideoSourceComponentPtr USBVideoSourceSystem::getUSBVideoSourceByName(const std::string& videoSourceName) const
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

USBVideoSourceComponentPtr USBVideoSourceSystem::addNewUSBVideoSource(
    const MikanUSBVideoSourceInfo& videoSourceInfo)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    MikanVideoSourceID videoSourceId = videoSourceSystemConfig->addUSBVideoSource(videoSourceInfo);
    if (videoSourceId != INVALID_MIKAN_ID)
    {
        USBVideoSourceDefinitionPtr configPtr = videoSourceSystemConfig->getUSBVideoSourceConfig(videoSourceId);
        assert(configPtr != nullptr);

        return createUSBVideoSourceObject(configPtr);
    }

    return USBVideoSourceComponentPtr();
}

bool USBVideoSourceSystem::removeUSBVideoSource(MikanVideoSourceID videoSourceId)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    return
        disposeUSBVideoSourceObject(videoSourceId) &&
        videoSourceSystemConfig->removeVideoSource(videoSourceId);
}

USBVideoSourceComponentPtr USBVideoSourceSystem::createUSBVideoSourceObject(
    USBVideoSourceDefinitionPtr videoSourceDefinition)
{
    MikanObjectPtr videoSourceObject = newObject();
    videoSourceObject->setName(videoSourceDefinition->getComponentName());

    // Make the USBVideoSource component the root of the object
    auto videoSourceComponentPtr = videoSourceObject->addComponent<USBVideoSourceComponent>();
    videoSourceComponentPtr->setDefinition(videoSourceDefinition);

    // Init the object once all components are added
    videoSourceObject->init();

    // Keep track of all the usb video sources in the system
    m_usbVideoSourceComponents.insert({ videoSourceDefinition->getVideoSourceId(), videoSourceComponentPtr });

    return videoSourceComponentPtr;
}

bool USBVideoSourceSystem::disposeUSBVideoSourceObject(MikanVideoSourceID videoSourceId)
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
