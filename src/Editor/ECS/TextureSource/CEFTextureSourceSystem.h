#pragma once

#include "ComponentFwd.h"
#include "MikanTypedObjectSystem.h"
#include "MikanTextureSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "CEFTextureSourceComponent.h"
#include "TextureSourceQueries.h"

#include <string>

class CEFTextureSourceSystemDefinition :
	public MikanTypedObjectSystemDefinition<CEFTextureSourceComponent, CEFTextureSourceDefinition, MikanTextureSourceID>
{
public:
	using Super = MikanTypedObjectSystemDefinition<CEFTextureSourceComponent, CEFTextureSourceDefinition, MikanTextureSourceID>;

	CEFTextureSourceSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

class CEFTextureSourceSystem :
	public MikanTypedObjectSystem<
		CEFTextureSourceComponent, CEFTextureSourceDefinition,
		MikanTextureSourceID,
		CEFTextureSourceSystem, CEFTextureSourceSystemDefinition>
{
public:
	using Super = MikanTypedObjectSystem<
		CEFTextureSourceComponent, CEFTextureSourceDefinition,
		MikanTextureSourceID,
		CEFTextureSourceSystem, CEFTextureSourceSystemDefinition>;

	CEFTextureSourceSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "CEFTextureSourceSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	TextureSourceComponentList getTextureSourceComponentList() const;
	TextureSourceIdList getTextureSourceIdList() const;
};

using CEFTextureSourceSystemPtr = std::shared_ptr<CEFTextureSourceSystem>;
