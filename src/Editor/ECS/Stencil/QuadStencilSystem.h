#pragma once

#include "ComponentFwd.h"
#include "MikanStencilTypes.h"
#include "MikanTypedObjectSystem.h"
#include "ObjectSystemConfigFwd.h"
#include "QuadStencilComponent.h"

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class QuadStencilSystemDefinition
	: public MikanTypedObjectSystemDefinition<QuadStencilComponent, QuadStencilDefinition, MikanStencilID>
{
public:
	using Super= MikanTypedObjectSystemDefinition<QuadStencilComponent, QuadStencilDefinition, MikanStencilID>;

	QuadStencilSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

protected:
};

class QuadStencilSystem : public MikanTypedObjectSystem<QuadStencilComponent, QuadStencilDefinition, MikanStencilID,
														QuadStencilSystem, QuadStencilSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<QuadStencilComponent, QuadStencilDefinition, MikanStencilID, QuadStencilSystem,
										QuadStencilSystemDefinition>;

	QuadStencilSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName= "QuadStencilSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	inline QuadStencilComponentPtr getQuadStencilById(MikanStencilID stencilId) const
	{
		return Super::getTypedComponentById(stencilId);
	}
	inline QuadStencilComponentPtr getQuadStencilByName(const std::string& stencilName) const
	{
		return Super::getTypedComponentByName(stencilName);
	}

	// Helper methods for compatibility
	void getRelevantQuadStencilList(const std::vector<MikanStencilID>* allowedStencilIds,
									const glm::vec3& cameraPosition, const glm::vec3& cameraForward,
									std::vector<QuadStencilComponentPtr>& outStencilList) const;

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

protected:
	static bool isStencilFacingCamera(StencilComponentConstPtr stencil, const glm::vec3& cameraPosition,
									  const glm::vec3& cameraForward);

	virtual void additionalComponentFactory(MikanObjectPtr ownerComponentObject,
											ComponentDefinitionPtr componentDefinition) override;
};
