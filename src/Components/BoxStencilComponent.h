#pragma once

#include "CommonConfig.h"
#include "StencilComponent.h"
#include "MikanClientTypes.h"
#include "ComponentFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/vector_float3.hpp"

class BoxStencilDefinition : public StencilComponentDefinition
{
public:
	BoxStencilDefinition();
	BoxStencilDefinition(const MikanStencilBox& box);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanStencilBox getBoxInfo() const;

	static const std::string k_boxStencilXSizePropertyId;
	float getBoxXSize() const { return m_boxSize.x; }
	void setBoxXSize(float size);

	static const std::string k_boxStencilYSizePropertyId;
	float getBoxYSize() const { return m_boxSize.y; }
	void setBoxYSize(float size);

	static const std::string k_boxStencilZSizePropertyId;
	float getBoxZSize() const { return m_boxSize.z; }
	void setBoxZSize(float size);

	void setBoxSize(float xSize, float ySize, float zSize);

private:
	MikanVector3f m_boxSize;
};

class BoxStencilComponent : public StencilComponent
{
public:
	BoxStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline BoxStencilDefinitionPtr getBoxStencilDefinition() const { return m_definition; }

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

protected:
	void updateBoxColliderExtents();

	BoxStencilDefinitionPtr m_definition;
	SelectionComponentWeakPtr m_selectionComponent;
	BoxColliderComponentWeakPtr m_boxCollider;
};