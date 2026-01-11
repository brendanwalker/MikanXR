#include "TextureSourceSettings/AppStage_TextureSourceSettings.h"
#include "CameraMath.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "TextureSourceComponent.h"

#include <easy/profiler.h>

// -- TextureSourceDefinition -----
const std::string TextureSourceDefinition::k_TextureSourceIdPropertyId = "texture_source_id";

TextureSourceDefinition::TextureSourceDefinition()
	: MikanComponentDefinition()
	, m_textureSourceId(INVALID_MIKAN_ID)
{}

TextureSourceDefinition::TextureSourceDefinition(
	MikanTextureSourceID textureSourceId,
	const std::string& textureSourceName)
	: MikanComponentDefinition(textureSourceId, textureSourceName)
	, m_textureSourceId(textureSourceId)
{}

configuru::Config TextureSourceDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt[k_TextureSourceIdPropertyId] = m_textureSourceId;

	return pt;
}

void TextureSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_textureSourceId = pt.get_or<MikanTextureSourceID>(k_TextureSourceIdPropertyId, m_textureSourceId);
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

// -- IPropertyInterface ----
void TextureSourceComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			TextureSourceDefinition::k_TextureSourceIdPropertyId, MikanVariantType::INT)
		->setReadOnly()
		->setDefaultValue(-1));
}

bool TextureSourceComponent::getPropertyValue(
	PropertyDescriptorConstPtr propertyDesc, 
	MikanVariant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == TextureSourceDefinition::k_TextureSourceIdPropertyId)
	{
		outValue = getTextureSourceId();
		return true;
	}

	return MikanComponent::getPropertyValue(propertyDesc, outValue);
}

bool TextureSourceComponent::setPropertyValue(
	PropertyDescriptorConstPtr propertyDesc, 
	const MikanVariant& inValue)
{
	return MikanComponent::setPropertyValue(propertyDesc, inValue);
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

void TextureSourceComponent::showTextureSourceSettings()
{
	getOwnerEditorWindow()->pushAppStageOfType<AppStage_TextureSourceSettings>()
		->setTextureSourceComponent(getSelfPtr<TextureSourceComponent>());
}