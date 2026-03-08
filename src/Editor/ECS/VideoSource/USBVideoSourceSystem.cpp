#include "USBVideoSourceSystem.h"
#include "USBVideoSourceComponent.h"
#include "IUsbVideoDeviceModule.h"
#include "Logger.h"
#include "MikanPropertyDatabase.h"
#include "MikanModuleManager.h"
#include "App.h"
#include "ProjectConfig.h"
#include "MikanObject.h"

#include <assert.h>

#define USB_VIDEO_DEVICE_MODULE_NAME "MikanWMFVideo"

// -- USBVideoSourceSystemDefinition -----
USBVideoSourceSystemDefinition::USBVideoSourceSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- USBVideoSourceSystem ----
USBVideoSourceSystem::USBVideoSourceSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

bool USBVideoSourceSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
    if (!createUsbVideoDeviceManager(USB_VIDEO_DEVICE_MODULE_NAME))
    {
        MIKAN_LOG_ERROR("USBVideoSourceSystem::init") <<
            "Failed to load USB video device module " << USB_VIDEO_DEVICE_MODULE_NAME;
		return false;
    }

	Super::init(definitionPtr);

    return true;
}

void USBVideoSourceSystem::dispose()
{
	Super::dispose();
	disposeUsbVideoDeviceManager();
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
	for (const auto& it : Super::getComponentMap())
	{
        USBVideoSourceComponentPtr componentPtr = it.second.lock();
		if (componentPtr)
		{
			videoSourceIdList.push_back(componentPtr->getVideoSourceId());
		}
	}
	return videoSourceIdList;
}

USBVideoSourceComponentPtr USBVideoSourceSystem::getUSBVideoSourceByPath(const std::string& videoSourcePath) const
{
    for (auto it = Super::getComponentMap().begin(); it != Super::getComponentMap().end(); it++)
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
	// If available, get the first connected device by default
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
			// Use base class method with custom definition init
			return Super::addNewObjectByTypedDefinition([usbVideoDevice](USBVideoSourceDefinitionPtr def) {
				const char* videoModeName = usbVideoDevice->getVideoModeName();

				def->setComponentName(usbVideoDevice->getFriendlyName());
				def->setDevicePath(usbVideoDevice->getDevicePath());
				def->setVideoMode(videoModeName ? videoModeName : "<INVALID>");

				return true;
			});
        }
	}

    return USBVideoSourceComponentPtr();
}

void USBVideoSourceSystem::onConnectedDeviceListChanged()
{
	// Broadcast any sources were disconnected
	if (OnVideoSourceListChanged)
	{
		OnVideoSourceListChanged();
	}
}