#pragma once

#include "ClientTextureSourceComponent.h"
#include "ComponentFwd.h"
#include "MikanTypedObjectSystem.h"
#include "MikanTextureSourceTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "TextureSourceQueries.h"

#include <string>

class ClientTextureSourceSystemDefinition : public MikanTypedObjectSystemDefinition<ClientTextureSourceComponent, ClientTextureSourceDefinition, MikanTextureSourceID>
{
public:
	using Super= MikanTypedObjectSystemDefinition<ClientTextureSourceComponent, ClientTextureSourceDefinition, MikanTextureSourceID>;

	ClientTextureSourceSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

class ClientTextureSourceSystem : public MikanTypedObjectSystem<
									  ClientTextureSourceComponent, ClientTextureSourceDefinition,
									  MikanTextureSourceID,
									  ClientTextureSourceSystem, ClientTextureSourceSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<
		ClientTextureSourceComponent, ClientTextureSourceDefinition,
		MikanTextureSourceID,
		ClientTextureSourceSystem, ClientTextureSourceSystemDefinition>;

	ClientTextureSourceSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName= "ClientTextureSourceSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	TextureSourceComponentList getTextureSourceComponentList() const;
	TextureSourceIdList getTextureSourceIdList() const;
};

using ClientTextureSourceSystemPtr= std::shared_ptr<ClientTextureSourceSystem>;
