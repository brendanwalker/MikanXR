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

#define USB_VIDEO_DEVICE_MODULE_NAME "MikanWMFVideo"

bool USBVideoSourceSystem::init()
{
	MikanObjectSystem::init();

    VideoSourceSystemConfigConstPtr videoSourceSystemConfig = 
        getProjectConfig()->videoSourceSystemConfig;

    if (!createUsbVideoDeviceManager(USB_VIDEO_DEVICE_MODULE_NAME))
    {
        MIKAN_LOG_ERROR("USBVideoSourceSystem::init") << 
            "Failed to load USB video device module " << USB_VIDEO_DEVICE_MODULE_NAME;
		return false;
    }

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
    {
		MIKAN_LOG_ERROR("USBVideoSourceSystem::createUsbVideoDeviceManager")
			<< "Missing USB video device module name";
		return false;
    }

	// Attempt to load the usb device module
	bool bSuccess = false;
    m_usbVideoDeviceModule = getMikanModuleManager()->getModule<IUsbVideoDeviceModule>(moduleName);
	if (m_usbVideoDeviceModule)
	{
		MIKAN_LOG_INFO("USBVideoSourceSystem::createUsbVideoDeviceManager")
			<< "Loaded module " << moduleName;

		// Attempt to create a device manager
        m_usbVideoDeviceManager = m_usbVideoDeviceModule->createUsbVideoDeviceManager();
		if (m_usbVideoDeviceManager)
		{
			MIKAN_LOG_INFO("USBVideoSourceSystem::createUsbVideoDeviceManager")
				<< "Allocated USB device manager for " << moduleName;

			// Attempt to startup the usb device manager
			if (m_usbVideoDeviceManager->startup())
			{
				MIKAN_LOG_INFO("USBVideoSourceSystem::createUsbVideoDeviceManager")
					<< "Started USBDeviceManger for " << moduleName;

				// Listen for device manager changes
				m_usbVideoDeviceManager->addListener(this);

				bSuccess = true;
			}
			else
			{
				MIKAN_LOG_WARNING("USBVideoSourceSystem::createUsbVideoDeviceManager")
					<< "Failed to startup UsbVideoDeviceManger for " << moduleName;
			}
		}
		else
		{
			MIKAN_LOG_WARNING("USBVideoSourceSystem::createUsbVideoDeviceManager")
				<< "Failed to allocate UsbVideoDeviceManger for " << moduleName;
		}
	}
	else
	{
		MIKAN_LOG_ERROR("USBVideoSourceSystem::createUsbVideoDeviceManager")
			<< "Failed to load module" << moduleName;
	}

	// Clean up if anything failed
	if (!bSuccess)
	{
		if (m_usbVideoDeviceManager)
		{
            m_usbVideoDeviceManager->shutdown();
            m_usbVideoDeviceManager = nullptr;
		}

		if (m_usbVideoDeviceModule)
		{
			getMikanModuleManager()->disposeModule(m_usbVideoDeviceModule);
            m_usbVideoDeviceModule = nullptr;
		}
	}

    return bSuccess;
}

void USBVideoSourceSystem::disposeUsbVideoDeviceManager()
{
    if (m_usbVideoDeviceManager)
    {
        m_usbVideoDeviceManager->removeListener(this);
        m_usbVideoDeviceManager->shutdown();
        m_usbVideoDeviceManager = nullptr;
    }

    if (m_usbVideoDeviceModule)
    {
        getMikanModuleManager()->disposeModule(m_usbVideoDeviceModule);
        m_usbVideoDeviceModule = nullptr;
    }
}

bool USBVideoSourceSystem::getConnectedUSBVideoSourcePaths(
    USBVideoSourcePathList& outVideoSourcePathList) const
{
	outVideoSourcePathList.clear();

    if (m_usbVideoDeviceManager)
    {
        for (size_t index = 0; index < m_usbVideoDeviceManager->getDeviceCount(); index++)
        {
            IUsbVideoDevice* usbVideoDevice= m_usbVideoDeviceManager->getDeviceByIndex(index);

            outVideoSourcePathList.push_back(usbVideoDevice->getDevicePath());
		}

        return true;
    }

	return false;
}

VideoSourceIdList USBVideoSourceSystem::getVideoSourceIdList() const
{
	VideoSourceIdList videoSourceIdList;
	for (const auto& it : m_usbVideoSourceComponents)
	{
        USBVideoSourceComponentPtr componentPtr = it.second.lock();
		if (componentPtr)
		{
			videoSourceIdList.push_back(componentPtr->getVideoSourceId());
		}
	}
	return videoSourceIdList;
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

USBVideoSourceComponentPtr USBVideoSourceSystem::getUSBVideoSourceByPath(const std::string& videoSourcePath) const
{
    for (auto it = m_usbVideoSourceComponents.begin(); it != m_usbVideoSourceComponents.end(); it++)
    {
        USBVideoSourceComponentPtr componentPtr = it->second.lock();
        if (componentPtr && componentPtr->getUSBVideoSourceDefinition()->getDevicePath() == videoSourcePath)
        {
            return componentPtr;
        }
    }

	return USBVideoSourceComponentPtr();
}

USBVideoSourceComponentPtr USBVideoSourceSystem::addNewUSBVideoSource()
{
	USBVideoSourceComponentPtr newVideoSourceComponentPtr;

	// If available, get the first connected device by default
    MikanUSBVideoSourceInfo videoSourceInfo = {};
    if (m_usbVideoDeviceManager && m_usbVideoDeviceManager->getDeviceCount() > 0)
    {
		// Find the first valid device with at least one video mode
		int firstValidDeviceIdex = -1;
        for (int deviceIndex = 0; deviceIndex < m_usbVideoDeviceManager->getDeviceCount(); deviceIndex++)
        {
            IUsbVideoDevice* testVideoDevice = m_usbVideoDeviceManager->getDeviceByIndex(deviceIndex);
            if (testVideoDevice && testVideoDevice->getAvailableVideoModesCount() > 0)
            {
                if (testVideoDevice->getVideoModeIndex() < 0)
                {
					testVideoDevice->setVideoModeByIndex(0);
                }

                firstValidDeviceIdex = deviceIndex;
                break;
			}
        }

        IUsbVideoDevice* usbVideoDevice = m_usbVideoDeviceManager->getDeviceByIndex(firstValidDeviceIdex);
        if (usbVideoDevice)
        {
            const char* videoModeName = usbVideoDevice->getVideoModeName();

            videoSourceInfo.usb_source_name.setValue(usbVideoDevice->getFriendlyName());
            videoSourceInfo.device_path.setValue(usbVideoDevice->getDevicePath());
            videoSourceInfo.video_mode.setValue(videoModeName ? videoModeName : "<INVALID>");
            videoSourceInfo.intrinsics.intrinsics_type= INVALID_CAMERA_INTRINSICS;
        }

        newVideoSourceComponentPtr = addNewUSBVideoSource(videoSourceInfo);
	}

    return newVideoSourceComponentPtr;
}

USBVideoSourceComponentPtr USBVideoSourceSystem::addNewUSBVideoSource(
    const MikanUSBVideoSourceInfo& videoSourceInfo)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = getProjectConfig()->videoSourceSystemConfig;

    return 
        createUSBVideoSourceObject(
            videoSourceSystemConfig->allocateUSBVideoSourceDefinition(videoSourceInfo));
}

bool USBVideoSourceSystem::removeUSBVideoSource(MikanVideoSourceID videoSourceId)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        getProjectConfig()->videoSourceSystemConfig;

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

	// Register the definition with the video source system
    getProjectConfig()->videoSourceSystemConfig->addUSBVideoSourceDefinition(videoSourceDefinition);

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

void USBVideoSourceSystem::onConnectedDeviceListChanged()
{	
	// Broadcast any sources were disconnected
	if (OnVideoSourceListChanged)
	{
		OnVideoSourceListChanged();
	}
}