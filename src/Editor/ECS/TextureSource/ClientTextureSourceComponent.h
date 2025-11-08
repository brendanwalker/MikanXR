#pragma once

#include "CompositorConstants.h"
#include "TextureSourceComponent.h"
#include "MkRendererFwd.h"

class ClientTextureSourceDefinition : public TextureSourceDefinition
{
public:
	ClientTextureSourceDefinition();
	ClientTextureSourceDefinition(
		MikanTextureSourceID TextureSourceId,
		const struct MikanClientTextureSourceInfo& TextureSourceInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_clientSourcePropertyId;
	inline const std::string& getClientSource() const { return m_clientSource; }
	void setClientSource(const std::string& clientSource);

private:
	std::string m_clientSource;
};

class ClientTextureSourceComponent : public TextureSourceComponent
{
public:
	ClientTextureSourceComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName = "ClientTextureSourceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline ClientTextureSourceDefinitionPtr getClientTextureSourceDefinition() const
	{
		return std::static_pointer_cast<ClientTextureSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;
	const std::string& getClientSourceName() const;

	// Texture Source Interface
	IMkTexturePtr getClientColorSourceTexture(eTextureSourceColorType clientTextureType) const;
	IMkTexturePtr getClientDepthSourceTexture(eTextureSourceDepthType clientTextureType) const;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static void getFunctionNamesStatic(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
	{ TextureSourceComponent::getRmlFunctionDescriptors(outDescriptors); }

protected:
	class ClientSourceManager* getClientSourceManager() const;

private:
	bool m_bIsClientSourceOpened = false;
	bool m_bIsClientSourceStreaming = false;
};