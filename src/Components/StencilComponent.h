	#pragma once

#include "MikanComponent.h"
#include "MikanClientTypes.h"
#include "FrameCompositorConstants.h"
#include "SceneComponent.h"

#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class StencilComponentDefinition : public SceneComponentDefinition
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

class StencilComponent : public SceneComponent
{
public:
	StencilComponent(MikanObjectWeakPtr owner);

	inline StencilComponentConfigPtr getStencilComponentDefinition() const { 
		return std::static_pointer_cast<StencilComponentDefinition>(m_definition); 
	}

	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	MikanStencilID getParentAnchorId() const;
	void attachSceneComponentToAnchor(MikanSpatialAnchorID newParentId);

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_deleteStencilFunctionId;
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	void deleteStencil();
};