#pragma once

#include "ComponentFwd.h"
#include "IUsbVideoDeviceManager.h"
#include "MikanTypedObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "USBVideoSourceComponent.h"
#include "VideoSourceQueries.h"

#include <string>

using USBVideoSourcePathList = std::vector<std::string>;

class USBVideoSourceSystemDefinition :
	public MikanTypedObjectSystemDefinition<USBVideoSourceComponent, USBVideoSourceDefinition, MikanVideoSourceID>
{
public:
	using Super = MikanTypedObjectSystemDefinition<USBVideoSourceComponent, USBVideoSourceDefinition, MikanVideoSourceID>;

	USBVideoSourceSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

class USBVideoSourceSystem :
	public MikanTypedObjectSystem<
		USBVideoSourceComponent, USBVideoSourceDefinition,
		MikanVideoSourceID,
		USBVideoSourceSystem, USBVideoSourceSystemDefinition>,
	public IUsbVideoDeviceManagerListener
{
public:
	using Super = MikanTypedObjectSystem<
		USBVideoSourceComponent, USBVideoSourceDefinition,
		MikanVideoSourceID,
		USBVideoSourceSystem, USBVideoSourceSystemDefinition>;

    USBVideoSourceSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "USBVideoSourceSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

    virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
    virtual void dispose() override;

	IUsbVideoDeviceManagerPtr getUSBVideoDeviceManager() const { return m_usbVideoDeviceManager; }

	bool getConnectedUSBVideoSourcePaths(USBVideoSourcePathList& outVideoSourcePathList) const;
    VideoSourceIdList getVideoSourceIdList() const;

    USBVideoSourceComponentPtr getUSBVideoSourceByPath(const std::string& videoSourcePath) const;
    USBVideoSourceComponentPtr addNewUSBVideoSource();

    MulticastDelegate<void()> OnVideoSourceListChanged;

protected:
    bool createUsbVideoDeviceManager(const std::string& moduleName);
    void disposeUsbVideoDeviceManager();

	// IUsbVideoDeviceManagerListener interface
	virtual void onConnectedDeviceListChanged() override;

private:
    class IUsbVideoDeviceModule* m_usbVideoDeviceModule = nullptr;
    IUsbVideoDeviceManagerPtr m_usbVideoDeviceManager = nullptr;
};