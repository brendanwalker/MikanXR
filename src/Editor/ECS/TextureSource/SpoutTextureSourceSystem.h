#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanTextureSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "TextureSourceSystemConfig.h"

#include <map>
#include <string>

using SpoutTextureSourceMap = std::map<MikanTextureSourceID, SpoutTextureSourceComponentWeakPtr>;

class SpoutTextureSourceSystem : public MikanObjectSystem
{
public:  
    SpoutTextureSourceSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

    virtual bool init() override;
    virtual void dispose() override;

    const SpoutTextureSourceMap& getSpoutTextureSourceMap() const { return m_spoutTextureSourceComponents; }
    TextureSourceIdList getTextureSourceIdList() const;
    SpoutTextureSourceComponentPtr getSpoutTextureSourceById(MikanTextureSourceID TextureSourceId) const;
    SpoutTextureSourceComponentPtr getSpoutTextureSourceByName(const std::string& TextureSourceName) const;
    SpoutTextureSourceComponentPtr addNewSpoutTextureSource();
    SpoutTextureSourceComponentPtr addNewSpoutTextureSource(const MikanSpoutTextureSourceInfo& TextureSourceInfo);
    bool removeSpoutTextureSource(MikanTextureSourceID TextureSourceId);

protected:
    SpoutTextureSourceComponentPtr createSpoutTextureSourceObject(SpoutTextureSourceDefinitionPtr sourceConfig);
    bool disposeSpoutTextureSourceObject(MikanTextureSourceID TextureSourceId);

private:
    SpoutTextureSourceMap m_spoutTextureSourceComponents;
};

using SpoutTextureSourceSystemPtr = std::shared_ptr<SpoutTextureSourceSystem>;
