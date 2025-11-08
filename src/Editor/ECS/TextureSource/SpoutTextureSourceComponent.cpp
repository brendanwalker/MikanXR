#include "SpoutTextureSourceComponent.h"
#include "StringUtils.h"
#include "MikanTextureSourceTypes.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- SpoutTextureSourceDefinition ------
const std::string SpoutTextureSourceDefinition::k_spoutSourcePropertyId = "spout_source";

SpoutTextureSourceDefinition::SpoutTextureSourceDefinition()
	: TextureSourceDefinition()
{}

SpoutTextureSourceDefinition::SpoutTextureSourceDefinition(
	MikanTextureSourceID TextureSourceId,
	const MikanSpoutTextureSourceInfo& TextureSourceInfo)
	: TextureSourceDefinition(
		TextureSourceId, 
		TextureSourceInfo.spout_source_name.getValue())
{}

configuru::Config SpoutTextureSourceDefinition::writeToJSON()
{
	configuru::Config pt = TextureSourceDefinition::writeToJSON();

	pt["spout_source"] = m_spoutSource;

	return pt;
}

void SpoutTextureSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	TextureSourceDefinition::readFromJSON(pt);

	m_spoutSource = pt.get_or<std::string>("spout_source", m_spoutSource);
}

void SpoutTextureSourceDefinition::setSpoutSource(const std::string& spoutSource)
{
	if (spoutSource != m_spoutSource)
	{
		m_spoutSource = spoutSource;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_spoutSourcePropertyId));
	}
}

// -- SpoutTextureSourceComponent -----
SpoutTextureSourceComponent::SpoutTextureSourceComponent(MikanObjectWeakPtr owner)
	: TextureSourceComponent(owner)
{}

void SpoutTextureSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	// Close any open video source that was open
	//closeTextureSource();
}

// Texture Source Interface
IMkTexturePtr SpoutTextureSourceComponent::getClientColorSourceTexture(
	eTextureSourceColorType textureSourceColorType) const
{
	return IMkTexturePtr();
}

// -- IRmlPropertyInterface ----
void SpoutTextureSourceComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	TextureSourceComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			SpoutTextureSourceDefinition::k_spoutSourcePropertyId));
}

bool SpoutTextureSourceComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == SpoutTextureSourceDefinition::k_spoutSourcePropertyId)
	{
		outValue = getSpoutTextureSourceDefinition()->getSpoutSource();
		return true;
	}

	return TextureSourceComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool SpoutTextureSourceComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == SpoutTextureSourceDefinition::k_spoutSourcePropertyId)
	{
		std::string devicePath = inValue.Get<std::string>();
		getSpoutTextureSourceDefinition()->setSpoutSource(devicePath);
		return true;
	}

	return TextureSourceComponent::setPropertyValueFromRml(propertyDesc, inValue);
}