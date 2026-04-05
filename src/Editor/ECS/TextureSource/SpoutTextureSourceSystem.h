#pragma once

#include "ComponentFwd.h"
#include "MikanTypedObjectSystem.h"
#include "MikanTextureSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "SpoutTextureSourceComponent.h"
#include "TextureSourceQueries.h"

#include <string>

class SpoutTextureSourceSystemDefinition :
	public MikanTypedObjectSystemDefinition<SpoutTextureSourceComponent, SpoutTextureSourceDefinition, MikanTextureSourceID>
{
public:
	using Super = MikanTypedObjectSystemDefinition<SpoutTextureSourceComponent, SpoutTextureSourceDefinition, MikanTextureSourceID>;

	SpoutTextureSourceSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

class SpoutTextureSourceSystem :
	public MikanTypedObjectSystem<
		SpoutTextureSourceComponent, SpoutTextureSourceDefinition,
		MikanTextureSourceID,
		SpoutTextureSourceSystem, SpoutTextureSourceSystemDefinition>
{
public:
	using Super = MikanTypedObjectSystem<
		SpoutTextureSourceComponent, SpoutTextureSourceDefinition,
		MikanTextureSourceID,
		SpoutTextureSourceSystem, SpoutTextureSourceSystemDefinition>;

    SpoutTextureSourceSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "SpoutTextureSourceSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
    virtual void dispose() override;

    TextureSourceComponentList getTextureSourceComponentList() const;
    TextureSourceIdList getTextureSourceIdList() const;

	void getAvailableSpoutSenderNames(std::vector<std::string>& outSenderNames) const;

private:
    struct SPOUTLIBRARY* m_spoutLibrary = nullptr;
};

using SpoutTextureSourceSystemPtr = std::shared_ptr<SpoutTextureSourceSystem>;
