#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "ObjectSystemConfigFwd.h"

#include <map>
#include <string>

using NetworkVideoSourceMap = std::map<MikanVideoSourceID, NetworkVideoSourceComponentWeakPtr>;

class NetworkVideoSourceSystem : public MikanObjectSystem
{
public:   
    NetworkVideoSourceSystem(class ObjectSystemManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

    virtual bool init() override;
    virtual void dispose() override;

    const NetworkVideoSourceMap& getNetworkVideoSourceMap() const { return m_networkVideoSourceComponents; }
    NetworkVideoSourceComponentPtr getNetworkVideoSourceById(MikanVideoSourceID videoSourceId) const;
    NetworkVideoSourceComponentPtr getNetworkVideoSourceByName(const std::string& videoSourceName) const;
    NetworkVideoSourceComponentPtr addNewNetworkVideoSource(const MikanNetworkVideoSourceInfo& videoSourceInfo);
    bool removeNetworkVideoSource(MikanVideoSourceID videoSourceId);

protected:
    NetworkVideoSourceComponentPtr createNetworkVideoSourceObject(NetworkVideoSourceDefinitionPtr sourceConfig);
    bool disposeNetworkVideoSourceObject(MikanVideoSourceID videoSourceId);

private:
    NetworkVideoSourceMap m_networkVideoSourceComponents;
};

using NetworkVideoSourceSystemPtr = std::shared_ptr<NetworkVideoSourceSystem>;
