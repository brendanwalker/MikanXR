#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "VideoSourceSystemConfig.h"

#include <map>
#include <string>

using ClientVideoSourceMap = std::map<MikanVideoSourceID, ClientVideoSourceComponentWeakPtr>;

class ClientVideoSourceSystem : public MikanObjectSystem
{
public:   
    ClientVideoSourceSystem(class ObjectSystemManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

    virtual bool init() override;
    virtual void dispose() override;

    const ClientVideoSourceMap& getClientVideoSourceMap() const { return m_clientVideoSourceComponents; }
    VideoSourceIdList getVideoSourceIdList() const;
    ClientVideoSourceComponentPtr getClientVideoSourceById(MikanVideoSourceID videoSourceId) const;
    ClientVideoSourceComponentPtr getClientVideoSourceByName(const std::string& videoSourceName) const;
    ClientVideoSourceComponentPtr addNewClientVideoSource();
    ClientVideoSourceComponentPtr addNewClientVideoSource(const MikanClientVideoSourceInfo& videoSourceInfo);
    bool removeClientVideoSource(MikanVideoSourceID videoSourceId);

protected:
    ClientVideoSourceComponentPtr createClientVideoSourceObject(ClientVideoSourceDefinitionPtr sourceConfig);
    bool disposeClientVideoSourceObject(MikanVideoSourceID videoSourceId);

private:
    ClientVideoSourceMap m_clientVideoSourceComponents;
};

using ClientVideoSourceSystemPtr = std::shared_ptr<ClientVideoSourceSystem>;
