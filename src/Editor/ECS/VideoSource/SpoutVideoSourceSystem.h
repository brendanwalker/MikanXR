#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "VideoSourceSystemConfig.h"

#include <map>
#include <string>

using SpoutVideoSourceMap = std::map<MikanVideoSourceID, SpoutVideoSourceComponentWeakPtr>;

class SpoutVideoSourceSystem : public MikanObjectSystem
{
public:  
    SpoutVideoSourceSystem(class ObjectSystemManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

    virtual bool init() override;
    virtual void dispose() override;

    const SpoutVideoSourceMap& getSpoutVideoSourceMap() const { return m_spoutVideoSourceComponents; }
    VideoSourceIdList getVideoSourceIdList() const;
    SpoutVideoSourceComponentPtr getSpoutVideoSourceById(MikanVideoSourceID videoSourceId) const;
    SpoutVideoSourceComponentPtr getSpoutVideoSourceByName(const std::string& videoSourceName) const;
    SpoutVideoSourceComponentPtr addNewSpoutVideoSource(const MikanSpoutVideoSourceInfo& videoSourceInfo);
    bool removeSpoutVideoSource(MikanVideoSourceID videoSourceId);

protected:
    SpoutVideoSourceComponentPtr createSpoutVideoSourceObject(SpoutVideoSourceDefinitionPtr sourceConfig);
    bool disposeSpoutVideoSourceObject(MikanVideoSourceID videoSourceId);

private:
    SpoutVideoSourceMap m_spoutVideoSourceComponents;
};

using SpoutVideoSourceSystemPtr = std::shared_ptr<SpoutVideoSourceSystem>;
