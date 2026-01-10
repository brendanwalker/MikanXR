#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanTextureSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "TextureSourceQueries.h"

#include <map>
#include <string>

using SpoutTextureSourceMap = std::map<MikanTextureSourceID, SpoutTextureSourceComponentWeakPtr>;

class SpoutTextureSourceSystemConfig : public MikanObjectSystemDefinition
{
public:
	SpoutTextureSourceSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_spoutTextureSourceListPropertyId;
	const std::vector<SpoutTextureSourceDefinitionPtr>& getSpoutTextureSourceList() const { return m_spoutTextureSourceList; }
	SpoutTextureSourceDefinitionConstPtr getSpoutTextureSourceConfigConst(MikanTextureSourceID textureSourceId) const;
	SpoutTextureSourceDefinitionPtr getSpoutTextureSourceConfig(MikanTextureSourceID textureSourceId);
	SpoutTextureSourceDefinitionPtr allocateSpoutTextureSourceDefinition(const struct MikanSpoutTextureSourceInfo& textureSourceInfo);
	bool addSpoutTextureSourceDefinition(SpoutTextureSourceDefinitionPtr textureSourcePtr);
	bool removeSpoutTextureSourceDefinition(MikanTextureSourceID textureSourceId);

protected:
	std::vector<SpoutTextureSourceDefinitionPtr> m_spoutTextureSourceList;
	MikanTextureSourceID m_nextTextureSourceId = 0;
};

class SpoutTextureSourceSystem : public MikanObjectSystem
{
public:  
    SpoutTextureSourceSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "SpoutTextureSourceSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	SpoutTextureSourceSystemConfigConstPtr getSpoutTextureSourceSystemConfigConst() const;
	SpoutTextureSourceSystemConfigPtr getSpoutTextureSourceSystemConfig();

    virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
    virtual void dispose() override;
    virtual MikanComponentPtr getComponentById(int componentId) const override;

    const SpoutTextureSourceMap& getSpoutTextureSourceMap() const { return m_spoutTextureSourceComponents; }
    TextureSourceComponentList getTextureSourceComponentList() const;
    TextureSourceIdList getTextureSourceIdList() const;
    SpoutTextureSourceComponentPtr getSpoutTextureSourceById(MikanTextureSourceID TextureSourceId) const;
    SpoutTextureSourceComponentPtr getSpoutTextureSourceByName(const std::string& TextureSourceName) const;
    SpoutTextureSourceComponentPtr addNewSpoutTextureSource();
    SpoutTextureSourceComponentPtr addNewSpoutTextureSource(const MikanSpoutTextureSourceInfo& TextureSourceInfo);
    bool removeSpoutTextureSource(MikanTextureSourceID TextureSourceId);
	void getAvailableSpoutSenderNames(std::vector<std::string>& outSenderNames) const;

    virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
    SpoutTextureSourceComponentPtr createSpoutTextureSourceObject(SpoutTextureSourceDefinitionPtr sourceConfig);
    bool disposeSpoutTextureSourceObject(MikanTextureSourceID TextureSourceId);

private:
    struct SPOUTLIBRARY* m_spoutLibrary= nullptr;
    SpoutTextureSourceMap m_spoutTextureSourceComponents;
    RmlDataBinding_SpoutSourceListPtr m_spoutSourceList;
};

using SpoutTextureSourceSystemPtr = std::shared_ptr<SpoutTextureSourceSystem>;
