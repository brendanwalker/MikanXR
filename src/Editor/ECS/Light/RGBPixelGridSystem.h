#pragma once

#include "ComponentFwd.h"
#include "LightSystemFwd.h"
#include "MikanLightTypes.h"
#include "MikanTypedObjectSystem.h"
#include "ObjectSystemConfigFwd.h"
#include "RGBPixelGridComponent.h"

#include <memory>
#include <string>

class IDMXManager;

// -- RGBPixelGridSystemDefinition -----
class RGBPixelGridSystemDefinition
	: public MikanTypedObjectSystemDefinition<RGBPixelGridComponent, RGBPixelGridDefinition, MikanLightID>
{
public:
	using Super= MikanTypedObjectSystemDefinition<RGBPixelGridComponent, RGBPixelGridDefinition, MikanLightID>;

	RGBPixelGridSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

// -- RGBPixelGridSystem -----
class RGBPixelGridSystem : public MikanTypedObjectSystem<RGBPixelGridComponent, RGBPixelGridDefinition, MikanLightID,
														 RGBPixelGridSystem, RGBPixelGridSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<RGBPixelGridComponent, RGBPixelGridDefinition, MikanLightID, RGBPixelGridSystem,
										RGBPixelGridSystemDefinition>;

	RGBPixelGridSystem(ProjectManagerPtr ownerProjectManager);

	inline static const std::string k_objectSystemClassName= "RGBPixelGridSystem";
	virtual std::string getObjectSystemClassName() const override { return k_objectSystemClassName; }

	virtual void update(float deltaSeconds) override;

	inline RGBPixelGridComponentPtr getGridById(MikanLightID gridId) const
	{
		return Super::getTypedComponentById(gridId);
	}
	inline RGBPixelGridComponentPtr getGridByName(const std::string& name) const
	{
		return Super::getTypedComponentByName(name);
	}
};
