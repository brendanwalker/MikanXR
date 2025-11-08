#include "TextureSourceSystem.h"
#include "CameraComponent.h"
#include "ClientTextureSourceSystem.h"
#include "ClientTextureSourceComponent.h"
#include "CameraObjectSystem.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "MikanObject.h"
#include "ProjectManager.h"
#include "SceneObjectSystem.h"
#include "SceneComponent.h"
#include "SpoutTextureSourceSystem.h"
#include "SpoutTextureSourceComponent.h"
#include "App.h"
#include "ProjectConfig.h"

TextureSourceSystemWeakPtr TextureSourceSystem::s_TextureSourceSystem;

bool TextureSourceSystem::init()
{
	MikanObjectSystem::init();

	// Create subsystems
	auto* owner= getOwnerProjectManager();
	m_clientTextureSourceSystem = owner->getSystemOfType<ClientTextureSourceSystem>();
	m_spoutTextureSourceSystem = owner->getSystemOfType<SpoutTextureSourceSystem>();

	s_TextureSourceSystem = std::static_pointer_cast<TextureSourceSystem>(shared_from_this());
	return true;
}

void TextureSourceSystem::dispose()
{
	s_TextureSourceSystem.reset();

	m_clientTextureSourceSystem.reset();
	m_spoutTextureSourceSystem.reset();

	MikanObjectSystem::dispose();
}

void TextureSourceSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	auto clientTextureSource = objectPtr->getComponentOfType<ClientTextureSourceComponent>();
	if (clientTextureSource != nullptr)
	{
		m_clientTextureSourceSystem->removeClientTextureSource(
			clientTextureSource->getTextureSourceDefinition()->getTextureSourceId());
		return;
	}

	auto spoutTextureSource = objectPtr->getComponentOfType<SpoutTextureSourceComponent>();
	if (spoutTextureSource != nullptr)
	{
		m_spoutTextureSourceSystem->removeSpoutTextureSource(
			spoutTextureSource->getTextureSourceDefinition()->getTextureSourceId());
		return;
	}
}

TextureSourceSystemConfigConstPtr TextureSourceSystem::getTextureSourceSystemConfigConst() const
{
	return getProjectConfig()->textureSourceSystemConfig;
}

TextureSourceSystemConfigPtr TextureSourceSystem::getTextureSourceSystemConfig()
{
	return std::const_pointer_cast<TextureSourceSystemConfig>(getTextureSourceSystemConfigConst());
}

TextureSourceComponentList TextureSourceSystem::getTextureSourceComponentList() const
{
	TextureSourceComponentList componentList;

	auto clientTextureSourceIds = m_clientTextureSourceSystem->getTextureSourceComponentList();
	componentList.insert(componentList.end(), clientTextureSourceIds.begin(), clientTextureSourceIds.end());
	auto spoutTextureSourceIds = m_spoutTextureSourceSystem->getTextureSourceComponentList();
	componentList.insert(componentList.end(), spoutTextureSourceIds.begin(), spoutTextureSourceIds.end());

	return componentList;
}

TextureSourceIdList TextureSourceSystem::getTextureSourceIdList() const
{
	TextureSourceIdList textureSourceIdList;
	
	auto clientTextureSourceIds = m_clientTextureSourceSystem->getTextureSourceIdList();
	textureSourceIdList.insert(textureSourceIdList.end(), clientTextureSourceIds.begin(), clientTextureSourceIds.end());
	auto spoutTextureSourceIds = m_spoutTextureSourceSystem->getTextureSourceIdList();
	textureSourceIdList.insert(textureSourceIdList.end(), spoutTextureSourceIds.begin(), spoutTextureSourceIds.end());

	return textureSourceIdList;
}

TextureSourceComponentPtr TextureSourceSystem::getTextureSourceById(MikanTextureSourceID TextureSourceId) const
{
	auto clientTextureSourcePtr = m_clientTextureSourceSystem->getClientTextureSourceById(TextureSourceId);
	if (clientTextureSourcePtr)
	{
		return clientTextureSourcePtr;
	}

	auto spoutTextureSourcePtr = m_spoutTextureSourceSystem->getSpoutTextureSourceById(TextureSourceId);
	if (spoutTextureSourcePtr)
	{
		return spoutTextureSourcePtr;
	}

	return TextureSourceComponentPtr();
}

eTextureSourceType TextureSourceSystem::getTextureSourceType(MikanTextureSourceID TextureSourceId) const
{
	auto clientTextureSourcePtr = m_clientTextureSourceSystem->getClientTextureSourceById(TextureSourceId);
	if (clientTextureSourcePtr)
	{
		return eTextureSourceType::client;
	}

	auto spoutTextureSourcePtr = m_spoutTextureSourceSystem->getSpoutTextureSourceById(TextureSourceId);
	if (spoutTextureSourcePtr)
	{
		return eTextureSourceType::spout;
	}

	return eTextureSourceType::INVALID;
}

bool TextureSourceSystem::removeTextureSource(MikanTextureSourceID TextureSourceId)
{
	switch (getTextureSourceType(TextureSourceId))
	{
		case eTextureSourceType::client:
			return m_clientTextureSourceSystem->removeClientTextureSource(TextureSourceId);
			break;
		case eTextureSourceType::spout:
			return m_spoutTextureSourceSystem->removeSpoutTextureSource(TextureSourceId);
			break;
	}

	return false;
}