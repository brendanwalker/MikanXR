#pragma once

#include "ComponentFwd.h"
#include "IUsbVideoDeviceManager.h"
#include "MikanTypedObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "USBVideoSourceComponent.h"
#include "VideoSourceQueries.h"

#include <map>
#include <string>

using USBVideoSourcePathList = std::vector<std::string>;
using USBVideoSourcePathMap = std::map<std::string, std::string>; // path -> friendly name

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
	bool getConnectedUSBVideoSourcePathMap(USBVideoSourcePathMap& outVideoSourcePathMap) const;
    VideoSourceIdList getVideoSourceIdList() const;

    USBVideoSourceComponentPtr getUSBVideoSourceByPath(const std::string& videoSourcePath) const;
    USBVideoSourceComponentPtr addNewUSBVideoSource();

    MulticastDelegate<void()> OnVideoSourceListChanged;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static const std::string k_usbDeviceMapPropertyId;
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;

protected:
    bool createUsbVideoDeviceManager(const std::string& moduleName);
    void disposeUsbVideoDeviceManager();

	// IUsbVideoDeviceManagerListener interface
	virtual void onConnectedDeviceListChanged() override;

private:
    class IUsbVideoDeviceModule* m_usbVideoDeviceModule = nullptr;
    IUsbVideoDeviceManagerPtr m_usbVideoDeviceManager = nullptr;
};