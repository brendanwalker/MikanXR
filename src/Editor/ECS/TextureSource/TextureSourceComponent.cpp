#include "TextureSourceSettings/AppStage_TextureSourceSettings.h"
#include "CameraMath.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "TextureSourceComponent.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <easy/profiler.h>

// -- TextureSourceDefinition -----
const std::string TextureSourceDefinition::k_TextureSourceIdPropertyId = "texture_source_id";

TextureSourceDefinition::TextureSourceDefinition()
	: MikanComponentDefinition()
	, m_TextureSourceId(INVALID_MIKAN_ID)
{}

TextureSourceDefinition::TextureSourceDefinition(
	MikanTextureSourceID TextureSourceId,
	const std::string& TextureSourceName)
	: MikanComponentDefinition(TextureSourceId, TextureSourceName)
	, m_TextureSourceId(TextureSourceId)
{}

configuru::Config TextureSourceDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt[k_TextureSourceIdPropertyId] = m_TextureSourceId;

	return pt;
}

void TextureSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_TextureSourceId = pt.get_or<MikanTextureSourceID>("video_source_id", m_TextureSourceId);
}

// -- TextureSourceComponent -----
TextureSourceComponent::TextureSourceComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
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

IMkTexturePtr TextureSourceComponent::getClientColorSourceTexture(eTextureSourceColorType textureSourceColorType) const
{
	return IMkTexturePtr();
}

IMkTexturePtr TextureSourceComponent::getClientDepthSourceTexture(eTextureSourceDepthType textureSourceColorType) const
{
	return IMkTexturePtr();
}

// -- IRmlPropertyInterface ----
void TextureSourceComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlPropertyDescriptors(outDescriptors);
}

bool TextureSourceComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc, 
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == TextureSourceDefinition::k_TextureSourceIdPropertyId)
	{
		outValue = getTextureSourceId();
		return true;
	}

	return MikanComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool TextureSourceComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc, 
	const Rml::Variant& inValue)
{
	return MikanComponent::setPropertyValueFromRml(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
const std::string TextureSourceComponent::k_deleteTextureSourceFunctionId = "delete_video_source";
const std::string TextureSourceComponent::k_showTextureSourceSettingsFunctionId = "show_texture_source_settings";

void TextureSourceComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_deleteTextureSourceFunctionId, "Delete Video Source"));
	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_showTextureSourceSettingsFunctionId, "Show Texture Source Settings"));
}

bool TextureSourceComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
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

	return MikanComponent::invokeFunctionFromRml(functionDesc);
}

void TextureSourceComponent::deleteTextureSource()
{
	getOwnerObject()->deleteSelfConfig();
}

void TextureSourceComponent::showTextureSourceSettings()
{
	getOwnerEditorWindow()->pushAppStageOfType<AppStage_TextureSourceSettings>()
		->setTextureSourceComponent(getSelfPtr<TextureSourceComponent>());
}