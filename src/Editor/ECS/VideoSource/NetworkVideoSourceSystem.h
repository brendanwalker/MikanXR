#pragma once

#include "ComponentFwd.h"
#include "INetworkVideoDeviceManager.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "VideoSourceSystemConfig.h"

#include <map>
#include <string>

using NetworkVideoSourceMap = std::map<MikanVideoSourceID, NetworkVideoSourceComponentWeakPtr>;

class NetworkVideoSourceSystem : public MikanObjectSystem
{
public:   
    NetworkVideoSourceSystem(class ObjectSystemManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

    virtual bool init() override;
	virtual void update(float deltaTime) override;
    virtual void dispose() override;

    INetworkVideoDeviceManagerPtr getNetworkVideoDeviceManager() const { return m_networkVideoDeviceManager; }

    const NetworkVideoSourceMap& getNetworkVideoSourceMap() const { return m_networkVideoSourceComponents; }
    VideoSourceIdList getVideoSourceIdList() const;
    NetworkVideoSourceComponentPtr getNetworkVideoSourceById(MikanVideoSourceID videoSourceId) const;
    NetworkVideoSourceComponentPtr getNetworkVideoSourceByName(const std::string& videoSourceName) const;
    NetworkVideoSourceComponentPtr addNewNetworkVideoSource();
    NetworkVideoSourceComponentPtr addNewNetworkVideoSource(const MikanNetworkVideoSourceInfo& videoSourceInfo);
    bool removeNetworkVideoSource(MikanVideoSourceID videoSourceId);

protected:
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
