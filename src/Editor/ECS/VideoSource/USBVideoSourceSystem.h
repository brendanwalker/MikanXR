#pragma once

#include "ComponentFwd.h"
#include "IUsbVideoDeviceManager.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "VideoSourceQueries.h"

#include <map>
#include <string>

using USBVideoSourcePathList = std::vector<std::string>;
using USBVideoSourceMap = std::map<MikanVideoSourceID, USBVideoSourceComponentWeakPtr>;

class USBVideoSourceSystemDefinition : public MikanObjectSystemDefinition
{
public:
	USBVideoSourceSystemDefinition(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_usbVideoSourceListPropertyId;
	const std::vector<USBVideoSourceDefinitionPtr>& getUSBVideoSourceList() const { return m_usbVideoSourceList; }
	USBVideoSourceDefinitionConstPtr getUSBVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	USBVideoSourceDefinitionPtr getUSBVideoSourceConfig(MikanVideoSourceID videoSourceId);
	USBVideoSourceDefinitionPtr allocateUSBVideoSourceDefinition(const struct MikanUSBVideoSourceInfo& videoSourceInfo);
	bool addUSBVideoSourceDefinition(USBVideoSourceDefinitionPtr videoSourcePtr);
	bool removeUSBVideoSourceDefinition(MikanVideoSourceID videoSourceId);

protected:
	std::vector<USBVideoSourceDefinitionPtr> m_usbVideoSourceList;
	MikanVideoSourceID m_nextVideoSourceId = 0;
};

class USBVideoSourceSystem : public MikanObjectSystem, public IUsbVideoDeviceManagerListener
{
public:
    USBVideoSourceSystem(ProjectManagerPtr ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "USBVideoSourceSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	USBVideoSourceSystemDefinitionConstPtr getUSBVideoSourceSystemConfigConst() const;
	USBVideoSourceSystemDefinitionPtr getUSBVideoSourceSystemConfig();

    virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
    virtual void dispose() override;
    virtual MikanComponentPtr getComponentById(int componentId) const override;

	IUsbVideoDeviceManagerPtr getUSBVideoDeviceManager() const { return m_usbVideoDeviceManager; }

    const USBVideoSourceMap& getUSBVideoSourceMap() const { return m_usbVideoSourceComponents; }
	bool getConnectedUSBVideoSourcePaths(USBVideoSourcePathList& outVideoSourcePathList) const;
    VideoSourceIdList getVideoSourceIdList() const;
    USBVideoSourceComponentPtr getUSBVideoSourceById(MikanVideoSourceID videoSourceId) const;
    USBVideoSourceComponentPtr getUSBVideoSourceByName(const std::string& videoSourceName) const;
    USBVideoSourceComponentPtr getUSBVideoSourceByPath(const std::string& videoSourcePath) const;
    USBVideoSourceComponentPtr addNewUSBVideoSource();
    USBVideoSourceComponentPtr addNewUSBVideoSource(const MikanUSBVideoSourceInfo& videoSourceInfo);
    bool removeUSBVideoSource(MikanVideoSourceID videoSourceId);

    MulticastDelegate<void()> OnVideoSourceListChanged;

    virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

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