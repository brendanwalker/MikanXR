#include "App.h"
#include "ClientTextureSourceComponent.h"
#include "SpoutTextureSourceComponent.h"
#include "TextureSourceSystemConfig.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "ProjectConfig.h"
#include "StringUtils.h"

// -- TextureSourceSystemConfig -----
const std::string TextureSourceSystemConfig::k_clientTextureSourceListPropertyId= "clientTextureSourceList";
const std::string TextureSourceSystemConfig::k_spoutTextureSourceListPropertyId= "spoutTextureSourceList";

configuru::Config TextureSourceSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["next_texture_source_id"] = m_nextTextureSourceId;

	std::vector<configuru::Config> clientTextureSourceConfigs;
	for (auto TextureSource : m_clientTextureSourceList)
	{
		clientTextureSourceConfigs.push_back(TextureSource->writeToJSON());
	}
	pt.insert_or_assign(k_clientTextureSourceListPropertyId, clientTextureSourceConfigs);

	std::vector<configuru::Config> spoutTextureSourceConfigs;
	for (auto TextureSource : m_spoutTextureSourceList)
	{
		spoutTextureSourceConfigs.push_back(TextureSource->writeToJSON());
	}
	pt.insert_or_assign(k_spoutTextureSourceListPropertyId, spoutTextureSourceConfigs);

	return pt;
}

void TextureSourceSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextTextureSourceId = pt.get_or<int>("next_texture_source_id", m_nextTextureSourceId);

	// Read in the client video sources
	m_clientTextureSourceList.clear();
	if (pt.has_key(k_clientTextureSourceListPropertyId))
	{
		for (const configuru::Config& TextureSource_pt : pt[k_clientTextureSourceListPropertyId].as_array())
		{
			auto definitionPtr = std::make_shared<ClientTextureSourceDefinition>();

			definitionPtr->readFromJSON(TextureSource_pt);
			m_clientTextureSourceList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}

	// Read in the spout video sources
	m_spoutTextureSourceList.clear();
	if (pt.has_key(k_spoutTextureSourceListPropertyId))
	{
		for (const configuru::Config& TextureSource_pt : pt[k_spoutTextureSourceListPropertyId].as_array())
		{
			auto definitionPtr = std::make_shared<SpoutTextureSourceDefinition>();

			definitionPtr->readFromJSON(TextureSource_pt);
			m_spoutTextureSourceList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

eTextureSourceType TextureSourceSystemConfig::getTextureSourceType(MikanTextureSourceID TextureSourceId) const
{
	auto clientTextureSourcePtr = getClientTextureSourceConfigConst(TextureSourceId);
	if (clientTextureSourcePtr)
	{
		return eTextureSourceType::client;
	}

	auto spoutTextureSourcePtr = getSpoutTextureSourceConfigConst(TextureSourceId);
	if (spoutTextureSourcePtr)
	{
		return eTextureSourceType::spout;
	}

	return eTextureSourceType::INVALID;
}

bool TextureSourceSystemConfig::removeTextureSource(MikanTextureSourceID TextureSourceId)
{
	switch (getTextureSourceType(TextureSourceId))
	{
		case eTextureSourceType::client:
			return removeClientTextureSource(TextureSourceId);
		case eTextureSourceType::spout:
			return removeSpoutTextureSource(TextureSourceId);
		default:
			return false;
	}
}

ClientTextureSourceDefinitionConstPtr TextureSourceSystemConfig::getClientTextureSourceConfigConst(
	MikanTextureSourceID TextureSourceId) const
{
	auto it = std::find_if(
		m_clientTextureSourceList.begin(), m_clientTextureSourceList.end(),
		[TextureSourceId](ClientTextureSourceDefinitionPtr configPtr) {
			return configPtr->getTextureSourceId() == TextureSourceId;
		});
	if (it != m_clientTextureSourceList.end())
	{
		return ClientTextureSourceDefinitionConstPtr(*it);
	}
	return ClientTextureSourceDefinitionConstPtr();
}

ClientTextureSourceDefinitionPtr TextureSourceSystemConfig::getClientTextureSourceConfig(
	MikanTextureSourceID TextureSourceId)
{
	auto constPtr = getClientTextureSourceConfigConst(TextureSourceId);
	if (constPtr)
	{
		return std::const_pointer_cast<ClientTextureSourceDefinition>(constPtr);
	}
	return ClientTextureSourceDefinitionPtr();
}

MikanTextureSourceID TextureSourceSystemConfig::addClientTextureSource(
	const MikanClientTextureSourceInfo& TextureSourceInfo)
{
	ClientTextureSourceDefinitionPtr TextureSourcePtr =
		std::make_shared<ClientTextureSourceDefinition>(m_nextTextureSourceId, TextureSourceInfo);
	m_nextTextureSourceId++;

	m_clientTextureSourceList.push_back(TextureSourcePtr);
	addChildConfig(TextureSourcePtr);

	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_clientTextureSourceListPropertyId));

	return TextureSourcePtr->getTextureSourceId();
}

bool TextureSourceSystemConfig::removeClientTextureSource(
	MikanTextureSourceID TextureSourceId)
{
	auto it = std::find_if(
		m_clientTextureSourceList.begin(), m_clientTextureSourceList.end(),
		[TextureSourceId](ClientTextureSourceDefinitionPtr configPtr) {
			return configPtr->getTextureSourceId() == TextureSourceId;
		});

	if (it != m_clientTextureSourceList.end())
	{
		removeChildConfig(*it);

		m_clientTextureSourceList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_clientTextureSourceListPropertyId));

		return true;
	}

	return false;
}

SpoutTextureSourceDefinitionConstPtr TextureSourceSystemConfig::getSpoutTextureSourceConfigConst(
	MikanTextureSourceID TextureSourceId) const
{
	auto it = std::find_if(
		m_spoutTextureSourceList.begin(), m_spoutTextureSourceList.end(),
		[TextureSourceId](SpoutTextureSourceDefinitionPtr configPtr) {
		return configPtr->getTextureSourceId() == TextureSourceId;
	});

	if (it != m_spoutTextureSourceList.end())
	{
		return SpoutTextureSourceDefinitionConstPtr(*it);
	}

	return SpoutTextureSourceDefinitionConstPtr();
}

SpoutTextureSourceDefinitionPtr TextureSourceSystemConfig::getSpoutTextureSourceConfig(
	MikanTextureSourceID TextureSourceId)
{
	return std::const_pointer_cast<SpoutTextureSourceDefinition>(
		getSpoutTextureSourceConfigConst(TextureSourceId));
}

MikanTextureSourceID TextureSourceSystemConfig::addSpoutTextureSource(
	const MikanSpoutTextureSourceInfo& TextureSourceInfo)
{
	SpoutTextureSourceDefinitionPtr TextureSourcePtr =
		std::make_shared<SpoutTextureSourceDefinition>(m_nextTextureSourceId, TextureSourceInfo);
	m_nextTextureSourceId++;

	m_spoutTextureSourceList.push_back(TextureSourcePtr);
	addChildConfig(TextureSourcePtr);

	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_spoutTextureSourceListPropertyId));

	return TextureSourcePtr->getTextureSourceId();
}

bool TextureSourceSystemConfig::removeSpoutTextureSource(MikanTextureSourceID TextureSourceId)
{
	auto it = std::find_if(
		m_spoutTextureSourceList.begin(), m_spoutTextureSourceList.end(),
		[TextureSourceId](SpoutTextureSourceDefinitionPtr configPtr) {
		return configPtr->getTextureSourceId() == TextureSourceId;
	});

	if (it != m_spoutTextureSourceList.end())
	{
		removeChildConfig(*it);

		m_spoutTextureSourceList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_spoutTextureSourceListPropertyId));

		return true;
	}

	return false;
}