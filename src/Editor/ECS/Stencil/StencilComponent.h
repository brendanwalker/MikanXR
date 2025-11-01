#pragma once

#include "MikanComponent.h"
#include "MikanStencilTypes.h"
#include "CompositorConstants.h"
#include "TransformComponent.h"

#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class StencilComponentDefinition : public TransformComponentDefinition
{
public:
	StencilComponentDefinition();
	StencilComponentDefinition(
		MikanStencilID stencilId,
		MikanSpatialAnchorID parentAnchorId,
		const std::string & componentName, 
		const MikanTransform& xform);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanStencilID getStencilId() const { return m_stencilId; }

	static const std::string k_parentAnchorPropertyId;
	MikanStencilID getParentAnchorId() const { return m_parentAnchorId; }
	void setParentAnchorId(MikanSpatialAnchorID anchorId);

	static const std::string k_stencilDisabledPropertyId;
	bool getIsDisabled() const { return m_bIsDisabled; }
	void setIsDisabled(bool flag);

	static const std::string k_stencilCullModePropertyId;
	eStencilCullMode getCullMode() const { return m_cullMode; }
	void setCullMode(eStencilCullMode mode);

protected:
	MikanStencilID m_stencilId= INVALID_MIKAN_ID;
	MikanSpatialAnchorID m_parentAnchorId= INVALID_MIKAN_ID;
	bool m_bIsDisabled= false;
	eStencilCullMode m_cullMode= eStencilCullMode::none;
};

class StencilComponent : public TransformComponent
{
public:
	StencilComponent(MikanObjectWeakPtr owner);

	inline StencilComponentConfigPtr getStencilComponentDefinition() const { 
		return std::static_pointer_cast<StencilComponentDefinition>(m_definition); 
	}

	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	inline static const std::string k_componentClassName = "StencilComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	void attachTransformComponentToAnchor(MikanSpatialAnchorID newParentId);

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static const std::string k_deleteStencilFunctionId;
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc) override;

	void deleteStencil();

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);
};