#pragma once

#include "ComponentFwd.h"
#include "DMXManagerConfig.h"
#include "LightSystemFwd.h"
#include "MikanLightTypes.h"
#include "MikanTypedObjectSystem.h"
#include "ObjectSystemConfigFwd.h"
#include "RGBSpotLightComponent.h"

#include <memory>
#include <string>

// -- RGBSpotLightSystemDefinition -----
class RGBSpotLightSystemDefinition : public MikanTypedObjectSystemDefinition<RGBSpotLightComponent, RGBSpotLightDefinition, MikanLightID>
{
public:
	using Super= MikanTypedObjectSystemDefinition<RGBSpotLightComponent, RGBSpotLightDefinition, MikanLightID>;

	RGBSpotLightSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

// -- RGBSpotLightSystem -----
class RGBSpotLightSystem : public MikanTypedObjectSystem<
							   RGBSpotLightComponent, RGBSpotLightDefinition,
							   MikanLightID,
							   RGBSpotLightSystem, RGBSpotLightSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<
		RGBSpotLightComponent, RGBSpotLightDefinition,
		MikanLightID,
		RGBSpotLightSystem, RGBSpotLightSystemDefinition>;

	RGBSpotLightSystem(ProjectManagerPtr ownerProjectManager);

	inline static const std::string k_objectSystemClassName= "RGBSpotLightSystem";
	virtual std::string getObjectSystemClassName() const override { return k_objectSystemClassName; }

	virtual void update(float deltaSeconds) override;

	inline RGBSpotLightComponentPtr getLightById(MikanLightID lightId) const
	{
		return Super::getTypedComponentById(lightId);
	}
	inline RGBSpotLightComponentPtr getLightByName(const std::string& name) const
	{
		return Super::getTypedComponentByName(name);
	}

protected:
	virtual void additionalComponentFactory(
		MikanObjectPtr ownerComponentObject,
		ComponentDefinitionPtr componentDefinition) override;
};
