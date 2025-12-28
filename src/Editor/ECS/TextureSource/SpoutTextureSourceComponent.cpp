#include "IMkTexture.h"
#include "Logger.h"
#include "MikanTextureSourceTypes.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutLibrary.h"
#include "StringUtils.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <easy/profiler.h>

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
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_spoutSourcePropertyId));
	}
}

// -- SpoutTextureSourceComponent -----
SpoutTextureSourceComponent::SpoutTextureSourceComponent(MikanObjectWeakPtr owner)
	: TextureSourceComponent(owner)
	, m_spoutColorFrame(nullptr)
{
	m_bWantsUpdate = true;
}

void SpoutTextureSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	// re-open the texture source
	openTextureSource();
}

void SpoutTextureSourceComponent::onDefinitionMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	TextureSourceComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);

	if (changedPropertySet.hasPropertyName(SpoutTextureSourceDefinition::k_spoutSourcePropertyId))
	{
		openTextureSource();
	}
}

void SpoutTextureSourceComponent::update(float deltaSeconds)
{
	if (m_spoutColorFrame != nullptr)
	{
		EASY_BLOCK("SpoutTextureSource: receive color texture");

		if (!m_colorTexture || m_spoutColorFrame->IsUpdated())
		{
			// Free any existing texture
			if (m_colorTexture)
			{
				m_colorTexture->disposeTexture();
				m_colorTexture = nullptr;
			}

			// Allocate a new texture to match the spout source shared texture
			m_colorTexture= CreateMkTexture();
			m_colorTexture->setSize(m_spoutColorFrame->GetSenderWidth(), m_spoutColorFrame->GetSenderHeight());
			m_colorTexture->setTextureFormat(MK_RGBA);
			m_colorTexture->setBufferFormat(MK_RGBA);
			m_colorTexture->createTexture();
		}

		// TODO: This assumes we are reading into an OpenGL texture
		GLuint textureId= m_colorTexture->getGlTextureId();
		if (textureId != 0)
		{
			m_spoutColorFrame->ReceiveTexture(textureId, GL_TEXTURE_2D);
		}
	}
}

void SpoutTextureSourceComponent::dispose()
{
	closeTextureSource();

	TextureSourceComponent::dispose();
}

const std::string& SpoutTextureSourceComponent::getSpoutSourceName() const
{
	return getSpoutTextureSourceDefinition()->getSpoutSource();
}

void SpoutTextureSourceComponent::closeTextureSource()
{
	if (m_spoutColorFrame)
	{
		m_spoutColorFrame->Release();
		m_spoutColorFrame = nullptr;
	}

	if (m_colorTexture)
	{
		m_colorTexture->disposeTexture();
		m_colorTexture = nullptr;
	}
}

void SpoutTextureSourceComponent::openTextureSource()
{
	closeTextureSource();

	const std::string& spoutSourceName = getSpoutSourceName();

	if (!spoutSourceName.empty())
	{
		m_spoutColorFrame = GetSpout();
		if (m_spoutColorFrame != nullptr)
		{

			m_spoutColorFrame->EnableSpoutLog();
			m_spoutColorFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutColorFrame->SetReceiverName(spoutSourceName.c_str());
		}
		else
		{
			MIKAN_LOG_ERROR("SpoutTextureSourceComponent") << "Failed to open spout for sender: " << spoutSourceName;
		}

	}
}

// -- Texture Source Interface ---
IMkTexturePtr SpoutTextureSourceComponent::getClientColorSourceTexture(
	eTextureSourceColorType textureSourceColorType) const
{
	return m_colorTexture;
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