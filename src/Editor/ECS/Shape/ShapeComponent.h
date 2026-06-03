#pragma once

#include "ComponentFwd.h"
#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"
#include "MkRendererFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "TransformComponent.h"

#include <string>

class ShapeComponentDefinition : public TransformComponentDefinition
{
public:
	ShapeComponentDefinition();
	ShapeComponentDefinition(MikanShapeID shapeId);

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	static const std::string k_textureSourceIdPropertyId;
	MikanTextureSourceID getTextureSourceId() const { return m_textureSourceId; }
	void setTextureSourceId(MikanTextureSourceID id);

protected:
	MikanTextureSourceID m_textureSourceId = INVALID_MIKAN_ID;
};

class ShapeComponent : public TransformComponent
{
public:
	ShapeComponent(MikanObjectWeakPtr owner);

	inline ShapeComponentDefinitionPtr getShapeComponentDefinition() const
	{
		return std::static_pointer_cast<ShapeComponentDefinition>(m_definition);
	}

	inline static const std::string k_componentClassName = "ShapeComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	MikanTextureSourceID getTextureSourceId() const;
	void setTextureSourceId(MikanTextureSourceID id);

	// Returns the last texture fetched from the texture source (may be null)
	inline IMkTexturePtr getColorTexture() const { return m_colorTexture; }

	virtual void update(float deltaSeconds) override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

protected:
	virtual void onDefinitionMarkedDirty(
		CommonConfigPtr configPtr,
		const ConfigPropertyChangeSet& changedPropertySet) override;

	// Fetched each frame from the referenced TextureSourceComponent
	IMkTexturePtr m_colorTexture;
};
