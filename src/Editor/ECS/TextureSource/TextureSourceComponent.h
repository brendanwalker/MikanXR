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
	TextureSourceDefinition(MikanTextureSourceID TextureSourceId);

	inline MikanTextureSourceID getTextureSourceId() const { return getComponentId(); }
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
	virtual IMkTexturePtr getClientColorSourceTexture(MikanCameraID cameraId, eTextureSourceColorType textureSourceColorType) const;
	virtual IMkTexturePtr getClientDepthSourceTexture(MikanCameraID cameraId, eTextureSourceDepthType textureSourceColorType) const;
	
	// Video Source Events
	MulticastDelegate<void(TextureSourceComponentPtr TextureSource)> OnOpened;
	MulticastDelegate<void(TextureSourceComponentPtr TextureSource)> OnClosed;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IFunctionInterface ----
	static const std::string k_deleteTextureSourceFunctionId;
	static const std::string k_showTextureSourceSettingsFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	void deleteTextureSource();
	virtual void showTextureSourceSettings() {};
};