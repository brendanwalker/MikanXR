#include "CameraMath.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "MikanTextureSourceTypes.h"
#include "TextureSourceComponent.h"

#include <easy/profiler.h>

// -- TextureSourceDefinition -----
TextureSourceDefinition::TextureSourceDefinition()
	: MikanComponentDefinition()
{}

TextureSourceDefinition::TextureSourceDefinition(
	MikanTextureSourceID textureSourceId)
	: MikanComponentDefinition(textureSourceId, "")
{}

// -- TextureSourceComponent -----
TextureSourceComponent::TextureSourceComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

// -- IEntityAccessor ----
rfk::Struct const* TextureSourceComponent::getClientAPIValuesStructType() const
{
	return &MikanTextureSourceValues::staticGetArchetype();
}

void TextureSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	auto TextureSourceDefinitionPtr = std::static_pointer_cast<TextureSourceDefinition>(definition);
}

MikanTextureSourceID TextureSourceComponent::getTextureSourceId() const
{
	return getTextureSourceDefinition()->getTextureSourceId();
}

IMkTexturePtr TextureSourceComponent::getClientColorSourceTexture(MikanCameraID cameraId, eTextureSourceColorType textureSourceColorType) const
{
	return IMkTexturePtr();
}

IMkTexturePtr TextureSourceComponent::getClientDepthSourceTexture(MikanCameraID cameraId, eTextureSourceDepthType textureSourceColorType) const
{
	return IMkTexturePtr();
}

// -- IFunctionInterface ----
const std::string TextureSourceComponent::k_deleteTextureSourceFunctionId = "delete_video_source";
const std::string TextureSourceComponent::k_showTextureSourceSettingsFunctionId = "show_texture_source_settings";

void TextureSourceComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_deleteTextureSourceFunctionId, "Delete Video Source"));
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_showTextureSourceSettingsFunctionId, "Show Texture Source Settings"));
}

bool TextureSourceComponent::invokeFunction(FunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionName = functionDesc->getFunctionName();

	if (functionName == k_deleteTextureSourceFunctionId)
	{
		deleteTextureSource();
		return true;
	}
	else if (functionName == k_showTextureSourceSettingsFunctionId)
	{
		showTextureSourceSettings();
		return true;
	}

	return MikanComponent::invokeFunction(functionDesc);
}

void TextureSourceComponent::deleteTextureSource()
{
	getOwnerObject()->deleteSelfConfig();
}