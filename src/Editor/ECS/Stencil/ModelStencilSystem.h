#pragma once

#include "ComponentFwd.h"
#include "MikanStencilTypes.h"
#include "MikanTypedObjectSystem.h"
#include "ModelStencilComponent.h"
#include "ObjectSystemConfigFwd.h"

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class ModelStencilSystemDefinition
	: public MikanTypedObjectSystemDefinition<ModelStencilComponent, ModelStencilDefinition, MikanStencilID>
{
public:
	using Super= MikanTypedObjectSystemDefinition<ModelStencilComponent, ModelStencilDefinition, MikanStencilID>;

	ModelStencilSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
};

class ModelStencilSystem : public MikanTypedObjectSystem<ModelStencilComponent, ModelStencilDefinition, MikanStencilID,
														 ModelStencilSystem, ModelStencilSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<ModelStencilComponent, ModelStencilDefinition, MikanStencilID,
										ModelStencilSystem, ModelStencilSystemDefinition>;

	ModelStencilSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName= "ModelStencilSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	inline ModelStencilComponentPtr getModelStencilById(MikanStencilID stencilId) const
	{
		return Super::getTypedComponentById(stencilId);
	}
	inline ModelStencilComponentPtr getModelStencilByName(const std::string& stencilName) const
	{
		return Super::getTypedComponentByName(stencilName);
	}

	// Helper methods for compatibility
	void getRelevantModelStencilList(const std::vector<MikanStencilID>* allowedStencilIds,
									 const glm::vec3& cameraPosition, const glm::vec3& cameraForward,
									 std::vector<ModelStencilComponentPtr>& outStencilList) const;

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
