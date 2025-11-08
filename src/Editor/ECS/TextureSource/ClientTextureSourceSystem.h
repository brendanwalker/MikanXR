#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanTextureSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "TextureSourceSystemConfig.h"

#include <map>
#include <string>

using ClientTextureSourceMap = std::map<MikanTextureSourceID, ClientTextureSourceComponentWeakPtr>;

class ClientTextureSourceSystem : public MikanObjectSystem
{
public:   
    ClientTextureSourceSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

    virtual bool init() override;
    virtual void dispose() override;

    const ClientTextureSourceMap& getClientTextureSourceMap() const { return m_clientTextureSourceComponents; }
	TextureSourceComponentList getTextureSourceComponentList() const;
    TextureSourceIdList getTextureSourceIdList() const;
    ClientTextureSourceComponentPtr getClientTextureSourceById(MikanTextureSourceID TextureSourceId) const;
    ClientTextureSourceComponentPtr getClientTextureSourceByName(const std::string& TextureSourceName) const;
    ClientTextureSourceComponentPtr addNewClientTextureSource();
    ClientTextureSourceComponentPtr addNewClientTextureSource(const MikanClientTextureSourceInfo& TextureSourceInfo);
    bool removeClientTextureSource(MikanTextureSourceID TextureSourceId);

protected:
    ClientTextureSourceComponentPtr createClientTextureSourceObject(ClientTextureSourceDefinitionPtr sourceConfig);
    bool disposeClientTextureSourceObject(MikanTextureSourceID TextureSourceId);

private:
    ClientTextureSourceMap m_clientTextureSourceComponents;
};

using ClientTextureSourceSystemPtr = std::shared_ptr<ClientTextureSourceSystem>;
