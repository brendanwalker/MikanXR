#pragma once

#include "BoxStencilComponent.h"
#include "ComponentFwd.h"
#include "MikanStencilTypes.h"
#include "MikanTypedObjectSystem.h"
#include "ObjectSystemConfigFwd.h"

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class BoxStencilSystemDefinition :
	public MikanTypedObjectSystemDefinition<BoxStencilComponent, BoxStencilDefinition, MikanStencilID>
{
public:
	using Super = MikanTypedObjectSystemDefinition<BoxStencilComponent, BoxStencilDefinition, MikanStencilID>;

	BoxStencilSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
};

class BoxStencilSystem :
	public MikanTypedObjectSystem<
		BoxStencilComponent, BoxStencilDefinition,
		MikanStencilID,
		BoxStencilSystem, BoxStencilSystemDefinition>
{
public:
	using Super = MikanTypedObjectSystem<
		BoxStencilComponent, BoxStencilDefinition,
		MikanStencilID,
		BoxStencilSystem, BoxStencilSystemDefinition>;

	BoxStencilSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "BoxStencilSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	inline BoxStencilComponentPtr getBoxStencilById(MikanStencilID stencilId) const {
		return Super::getTypedComponentById(stencilId);
	}
	inline BoxStencilComponentPtr getBoxStencilByName(const std::string& stencilName) const {
		return Super::getTypedComponentByName(stencilName);
	}

	// Helper methods for compatibility
	void getRelevantBoxStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		const glm::vec3& cameraPosition,
		const glm::vec3& cameraForward,
		std::vector<BoxStencilComponentPtr>& outStencilList) const;

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

protected:
	static bool isStencilFacingCamera(
		StencilComponentConstPtr stencil,
		const glm::vec3& cameraPosition, const glm::vec3& cameraForward);

	virtual void additionalComponentFactory(
		MikanObjectPtr ownerComponentObject,
		ComponentDefinitionPtr componentDefinition) override;
};
