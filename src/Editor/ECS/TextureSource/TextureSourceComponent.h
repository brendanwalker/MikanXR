#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "CompositorConstants.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "MikanTextureSourceTypes.h"
#include "MkRendererFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "VideoDisplayConstants.h"

#include <map>
#include <memory>
#include <string>

class TextureSourceDefinition : public MikanComponentDefinition
{
public:
	TextureSourceDefinition();
	TextureSourceDefinition(
		MikanTextureSourceID TextureSourceId,
		const std::string& TextureSourceName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_TextureSourceIdPropertyId;
	inline MikanTextureSourceID getTextureSourceId() const { return m_TextureSourceId; }

private:
	MikanTextureSourceID m_TextureSourceId;
};

class TextureSourceComponent : public MikanComponent
{
public:
	TextureSourceComponent(MikanObjectWeakPtr owner);

	inline TextureSourceDefinitionPtr getTextureSourceDefinition() const
	{
		return std::static_pointer_cast<TextureSourceDefinition>(getDefinition());
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	inline static const std::string k_componentClassName = "TextureSourceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	// Texture Source Interface
	MikanTextureSourceID getTextureSourceId() const;
	virtual IMkTexturePtr getClientColorSourceTexture(eTextureSourceColorType clientTextureType) const;
	virtual IMkTexturePtr getClientDepthSourceTexture(eTextureSourceDepthType clientTextureType) const;
	
	// Video Source Events
	MulticastDelegate<void(TextureSourceComponentPtr TextureSource)> OnOpened;
	MulticastDelegate<void(TextureSourceComponentPtr TextureSource)> OnClosed;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static const std::string k_deleteTextureSourceFunctionId;
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc) override;

	void deleteTextureSource();
};