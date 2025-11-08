#pragma once

#include "TextureSourceComponent.h"

class SpoutTextureSourceDefinition : public TextureSourceDefinition
{
public:
	SpoutTextureSourceDefinition();
	SpoutTextureSourceDefinition(
		MikanTextureSourceID TextureSourceId,
		const struct MikanSpoutTextureSourceInfo& TextureSourceInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_spoutSourcePropertyId;
	inline const std::string& getSpoutSource() const { return m_spoutSource; }
	void setSpoutSource(const std::string& spoutSource);

private:
	std::string m_spoutSource;
};

class SpoutTextureSourceComponent : public TextureSourceComponent
{
public:
	SpoutTextureSourceComponent(MikanObjectWeakPtr owner);

	inline SpoutTextureSourceDefinitionPtr getSpoutTextureSourceDefinition() const
	{
		return std::static_pointer_cast<SpoutTextureSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	inline static const std::string k_componentClassName = "SpoutTextureSourceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	// Texture Source Interface
	virtual IMkTexturePtr getClientColorSourceTexture(eTextureSourceColorType clientTextureType) const;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
	{ TextureSourceComponent::getRmlFunctionDescriptors(outDescriptors); }
};