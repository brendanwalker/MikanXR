#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"
#include "NodeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class CompositorDefinition : public MikanComponentDefinition
{
public:
	CompositorDefinition();
	CompositorDefinition(
		MikanCompositorID compositorId,
		const std::string& compositorName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanCompositorID getCompositorId() const { return m_compositorId; }

	static const std::string k_compositorGraphPathPropertyId;
	bool hasCompositorGraphPath() const;
	const std::filesystem::path& getCompositorGraphPath() const;
	void setCompositorGraphPath(const std::filesystem::path& graphPath);

private:
	MikanCompositorID m_compositorId;
	MikanCameraID m_cameraId = INVALID_MIKAN_ID;
	AssetReferenceConfigPtr m_nodeGraphAssetRef;
};

class CompositorComponent : public MikanComponent
{
public:
	CompositorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

	inline CompositorDefinitionPtr getCompositorDefinition() const
	{
		return std::static_pointer_cast<CompositorDefinition>(m_definition);
	}
	const std::filesystem::path& getCompositorGraphAssetPath() const;
	void setCompositorGraphAssetPath(const std::filesystem::path& assetRefPath);

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

protected:
	// Compositor Node Graph
	CompositorNodeGraphPtr m_nodeGraph;
};
