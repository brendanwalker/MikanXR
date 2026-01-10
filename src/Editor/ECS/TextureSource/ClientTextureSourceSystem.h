#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanTextureSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "TextureSourceQueries.h"

#include <map>
#include <string>

using ClientTextureSourceMap = std::map<MikanTextureSourceID, ClientTextureSourceComponentWeakPtr>;

class ClientTextureSourceSystemConfig : public MikanObjectSystemDefinition
{
public:
	ClientTextureSourceSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_clientTextureSourceListPropertyId;
	const std::vector<ClientTextureSourceDefinitionPtr>& getClientTextureSourceList() const { return m_clientTextureSourceList; }
	ClientTextureSourceDefinitionConstPtr getClientTextureSourceConfigConst(MikanTextureSourceID textureSourceId) const;
	ClientTextureSourceDefinitionPtr getClientTextureSourceConfig(MikanTextureSourceID textureSourceId);
	ClientTextureSourceDefinitionPtr allocateClientTextureSourceDefinition(const struct MikanClientTextureSourceInfo& textureSourceInfo);
	bool addClientTextureSourceDefinition(ClientTextureSourceDefinitionPtr textureSourcePtr);
	bool removeClientTextureSourceDefinition(MikanTextureSourceID textureSourceId);

protected:
	std::vector<ClientTextureSourceDefinitionPtr> m_clientTextureSourceList;
	MikanTextureSourceID m_nextTextureSourceId = 0;
};

class ClientTextureSourceSystem : public MikanObjectSystem
{
public:   
    ClientTextureSourceSystem(ProjectManagerPtr ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "ClientTextureSourceSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	ClientTextureSourceSystemConfigConstPtr getClientTextureSourceSystemConfigConst() const;
	ClientTextureSourceSystemConfigPtr getClientTextureSourceSystemConfig();

    virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
    virtual void dispose() override;

    virtual MikanComponentPtr getComponentById(int componentId) const override;

    const ClientTextureSourceMap& getClientTextureSourceMap() const { return m_clientTextureSourceComponents; }
	TextureSourceComponentList getTextureSourceComponentList() const;
    TextureSourceIdList getTextureSourceIdList() const;
    ClientTextureSourceComponentPtr getClientTextureSourceById(MikanTextureSourceID TextureSourceId) const;
    ClientTextureSourceComponentPtr getClientTextureSourceByName(const std::string& TextureSourceName) const;
    ClientTextureSourceComponentPtr addNewClientTextureSource();
    ClientTextureSourceComponentPtr addNewClientTextureSource(const MikanClientTextureSourceInfo& TextureSourceInfo);
    bool removeClientTextureSource(MikanTextureSourceID TextureSourceId);

    virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
    ClientTextureSourceComponentPtr createClientTextureSourceObject(ClientTextureSourceDefinitionPtr sourceConfig);
    bool disposeClientTextureSourceObject(MikanTextureSourceID TextureSourceId);

private:
    ClientTextureSourceMap m_clientTextureSourceComponents;
};

using ClientTextureSourceSystemPtr = std::shared_ptr<ClientTextureSourceSystem>;
