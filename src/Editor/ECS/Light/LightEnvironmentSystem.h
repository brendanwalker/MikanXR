#pragma once

#include "ComponentFwd.h"
#include "LightEnvironmentComponent.h"
#include "LightSystemFwd.h"
#include "MikanLightTypes.h"
#include "MikanTypedObjectSystem.h"
#include "ObjectSystemConfigFwd.h"

#include <memory>
#include <string>

// -- LightEnvironmentSystemDefinition -----
class LightEnvironmentSystemDefinition
	: public MikanTypedObjectSystemDefinition<LightEnvironmentComponent, LightEnvironmentDefinition, MikanLightID>
{
public:
	using Super= MikanTypedObjectSystemDefinition<LightEnvironmentComponent, LightEnvironmentDefinition, MikanLightID>;

	LightEnvironmentSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

// -- LightEnvironmentSystem -----
/// Owns the scene lighting probes. Each probe carries an order-2 spherical
/// harmonic environment recovered from a captured frame; see
/// docs/reference/scene-lighting.md.
class LightEnvironmentSystem
	: public MikanTypedObjectSystem<LightEnvironmentComponent, LightEnvironmentDefinition, MikanLightID,
									LightEnvironmentSystem, LightEnvironmentSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<LightEnvironmentComponent, LightEnvironmentDefinition, MikanLightID,
										LightEnvironmentSystem, LightEnvironmentSystemDefinition>;

	LightEnvironmentSystem(ProjectManagerPtr ownerProjectManager);

	inline static const std::string k_objectSystemClassName= "LightEnvironmentSystem";
	virtual std::string getObjectSystemClassName() const override { return k_objectSystemClassName; }

	inline LightEnvironmentComponentPtr getLightEnvironmentById(MikanLightID lightId) const
	{
		return Super::getTypedComponentById(lightId);
	}
	inline LightEnvironmentComponentPtr getLightEnvironmentByName(const std::string& name) const
	{
		return Super::getTypedComponentByName(name);
	}
};
