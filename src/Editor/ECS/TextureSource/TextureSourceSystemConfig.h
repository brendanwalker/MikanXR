#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <configuru.hpp>

using TextureSourceIdList = std::vector<MikanTextureSourceID>;
using TextureSourceComponentList = std::vector<TextureSourceComponentPtr>;

class TextureSourceSystemConfig : public MikanObjectSystemDefinition
{
public:
	TextureSourceSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eTextureSourceType getTextureSourceType(MikanTextureSourceID TextureSourceId) const;
	bool removeTextureSource(MikanTextureSourceID TextureSourceId);

	static const std::string k_clientTextureSourceListPropertyId;
	const std::vector<ClientTextureSourceDefinitionPtr>& getClientTextureSourceList() const { return m_clientTextureSourceList; }
	ClientTextureSourceDefinitionConstPtr getClientTextureSourceConfigConst(MikanTextureSourceID TextureSourceId) const;
	ClientTextureSourceDefinitionPtr getClientTextureSourceConfig(MikanTextureSourceID TextureSourceId);
	ClientTextureSourceDefinitionPtr allocateClientTextureSourceDefinition(const struct MikanClientTextureSourceInfo& TextureSourceInfo);
	bool addClientTextureSourceDefinition(ClientTextureSourceDefinitionPtr TextureSourcePtr);
	bool removeClientTextureSourceDefinition(MikanTextureSourceID TextureSourceId);

	static const std::string k_spoutTextureSourceListPropertyId;
	const std::vector<SpoutTextureSourceDefinitionPtr>& getSpoutTextureSourceList() const { return m_spoutTextureSourceList; }
	SpoutTextureSourceDefinitionConstPtr getSpoutTextureSourceConfigConst(MikanTextureSourceID TextureSourceId) const;
	SpoutTextureSourceDefinitionPtr getSpoutTextureSourceConfig(MikanTextureSourceID TextureSourceId);
	SpoutTextureSourceDefinitionPtr allocateSpoutTextureSourceDefinition(const struct MikanSpoutTextureSourceInfo& TextureSourceInfo);
	bool addSpoutTextureSourceDefinition(SpoutTextureSourceDefinitionPtr TextureSourcePtr);
	bool removeSpoutTextureSourceDefinition(MikanTextureSourceID TextureSourceId);

protected:
	std::vector<ClientTextureSourceDefinitionPtr> m_clientTextureSourceList;
	std::vector<SpoutTextureSourceDefinitionPtr> m_spoutTextureSourceList;
	MikanTextureSourceID m_nextTextureSourceId = 0;
};