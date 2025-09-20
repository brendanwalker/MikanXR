#pragma once

#include "ComponentFwd.h"
#include "IUsbVideoDeviceManager.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "VideoSourceSystemConfig.h"

#include <map>
#include <string>

using USBVideoSourcePathList = std::vector<std::string>;
using USBVideoSourceMap = std::map<MikanVideoSourceID, USBVideoSourceComponentWeakPtr>;

class USBVideoSourceSystem : public MikanObjectSystem, public IUsbVideoDeviceManagerListener
{
public:
    USBVideoSourceSystem(class ObjectSystemManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

    virtual bool init() override;
    virtual void dispose() override;

	IUsbVideoDeviceManagerPtr getUSBVideoDeviceManager() const { return m_usbVideoDeviceManager; }

    const USBVideoSourceMap& getUSBVideoSourceMap() const { return m_usbVideoSourceComponents; }
	bool getConnectedUSBVideoSourcePaths(USBVideoSourcePathList& outVideoSourcePathList) const;
    VideoSourceIdList getVideoSourceIdList() const;
    USBVideoSourceComponentPtr getUSBVideoSourceById(MikanVideoSourceID videoSourceId) const;
    USBVideoSourceComponentPtr getUSBVideoSourceByName(const std::string& videoSourceName) const;
    USBVideoSourceComponentPtr addNewUSBVideoSource();
    USBVideoSourceComponentPtr addNewUSBVideoSource(const MikanUSBVideoSourceInfo& videoSourceInfo);
    bool removeUSBVideoSource(MikanVideoSourceID videoSourceId);

    MulticastDelegate<void()> OnVideoSourceListChanged;

protected:
    bool createUsbVideoDeviceManager(const std::string& moduleName);
    void disposeUsbVideoDeviceManager();

    USBVideoSourceComponentPtr createUSBVideoSourceObject(USBVideoSourceDefinitionPtr sourceConfig);
    bool disposeUSBVideoSourceObject(MikanVideoSourceID videoSourceId);

	// IUsbVideoDeviceManagerListener interface
	virtual void onConnectedDeviceListChanged() override;

private:
    class IUsbVideoDeviceModule* m_usbVideoDeviceModule = nullptr;
    IUsbVideoDeviceManagerPtr m_usbVideoDeviceManager = nullptr;
    USBVideoSourceMap m_usbVideoSourceComponents;
};