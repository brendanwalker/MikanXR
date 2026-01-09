#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanTextureSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "TextureSourceSystemConfig.h"
#include "Shared/RmlDataBinding_Fwd.h"

#include <map>
#include <string>

using SpoutTextureSourceMap = std::map<MikanTextureSourceID, SpoutTextureSourceComponentWeakPtr>;

class SpoutTextureSourceSystem : public MikanObjectSystem
{
public:  
    SpoutTextureSourceSystem(class ProjectManager* ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "SpoutTextureSourceSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

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
