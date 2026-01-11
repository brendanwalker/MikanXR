#pragma once

#include "ComponentFwd.h"
#include "INetworkVideoDeviceManager.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "VideoSourceQueries.h"

#include <map>
#include <string>

using NetworkVideoSourceMap = std::map<MikanVideoSourceID, NetworkVideoSourceComponentWeakPtr>;

class NetworkVideoSourceSystemDefinition : public MikanObjectSystemDefinition
{
public:
	NetworkVideoSourceSystemDefinition(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_networkedVideoSourceListPropertyId;
	const std::vector<NetworkVideoSourceDefinitionPtr>& getNetworkedVideoSourceList() const { return m_networkedVideoSourceList; }
	NetworkVideoSourceDefinitionConstPtr getNetworkedVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	NetworkVideoSourceDefinitionPtr getNetworkedVideoSourceConfig(MikanVideoSourceID videoSourceId);
	NetworkVideoSourceDefinitionPtr allocateNetworkedVideoSourceDefinition(const struct MikanNetworkVideoSourceInfo& videoSourceInfo);
	bool addNetworkedVideoSourceDefinition(NetworkVideoSourceDefinitionPtr videoSourcePtr);
	bool removeNetworkedVideoSourceDefinition(MikanVideoSourceID videoSourceId);

protected:
	std::vector<USBVideoSourceDefinitionPtr> m_usbVideoSourceList;
	std::vector<NetworkVideoSourceDefinitionPtr> m_networkedVideoSourceList;
	MikanVideoSourceID m_nextVideoSourceId = 0;
};

class NetworkVideoSourceSystem : public MikanObjectSystem
{
public:   
    NetworkVideoSourceSystem(ProjectManagerPtr ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "NetworkVideoObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	NetworkVideoSourceSystemDefinitionConstPtr getNetworkVideoSourceSystemConfigConst() const;
	NetworkVideoSourceSystemDefinitionPtr getNetworkVideoSourceSystemConfig();

    virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void update(float deltaTime) override;
    virtual void dispose() override;
    virtual MikanComponentPtr getComponentById(int componentId) const override;

    INetworkVideoDeviceManagerPtr getNetworkVideoDeviceManager() const { return m_networkVideoDeviceManager; }

    const NetworkVideoSourceMap& getNetworkVideoSourceMap() const { return m_networkVideoSourceComponents; }
    VideoSourceIdList getVideoSourceIdList() const;
    NetworkVideoSourceComponentPtr getNetworkVideoSourceById(MikanVideoSourceID videoSourceId) const;
    NetworkVideoSourceComponentPtr getNetworkVideoSourceByName(const std::string& videoSourceName) const;
    NetworkVideoSourceComponentPtr addNewNetworkVideoSource();
    NetworkVideoSourceComponentPtr addNewNetworkVideoSource(const MikanNetworkVideoSourceInfo& videoSourceInfo);
    bool removeNetworkVideoSource(MikanVideoSourceID videoSourceId);

    virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	bool ensureNetworkDeviceManager();
	bool createNetworkVideoDeviceManager(const std::string& moduleName);
	void disposeNetworkVideoDeviceManager();

    NetworkVideoSourceComponentPtr createNetworkVideoSourceObject(NetworkVideoSourceDefinitionPtr sourceConfig);
    bool disposeNetworkVideoSourceObject(MikanVideoSourceID videoSourceId);

private:
	class INetworkVideoDeviceModule* m_networkVideoDeviceModule = nullptr;
	INetworkVideoDeviceManagerPtr m_networkVideoDeviceManager = nullptr;
    NetworkVideoSourceMap m_networkVideoSourceComponents;
};

using NetworkVideoSourceSystemPtr = std::shared_ptr<NetworkVideoSourceSystem>;
