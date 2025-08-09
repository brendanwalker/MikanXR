#pragma once

#include "ComponentFwd.h"
#include "IUsbVideoDeviceManager.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "ObjectSystemConfigFwd.h"

#include <map>
#include <string>

using USBVideoSourceMap = std::map<MikanVideoSourceID, USBVideoSourceComponentWeakPtr>;

class USBVideoSourceSystem : public MikanObjectSystem, public IUsbVideoDeviceManagerListener
{
public:
    USBVideoSourceSystem(class ObjectSystemManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

    virtual bool init() override;
    virtual void dispose() override;

    const USBVideoSourceMap& getUSBVideoSourceMap() const { return m_usbVideoSourceComponents; }
    USBVideoSourceComponentPtr getUSBVideoSourceById(MikanVideoSourceID videoSourceId) const;
    USBVideoSourceComponentPtr getUSBVideoSourceByName(const std::string& videoSourceName) const;
    USBVideoSourceComponentPtr addNewUSBVideoSource(const MikanUSBVideoSourceInfo& videoSourceInfo);
    bool removeUSBVideoSource(MikanVideoSourceID videoSourceId);

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

using USBVideoSourceSystemPtr = std::shared_ptr<USBVideoSourceSystem>;
