#include "ClientTextureSourceComponent.h"
#include "ClientTextureSourceSystem.h"
#include "RmlModel_ProjectSources.h"
#include "MikanCoreTypes.h"
#include "NetworkVideoSourceComponent.h"
#include "NetworkVideoSourceSystem.h"
#include "ProjectConfig.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlModel_TextureSourceComponent.h"
#include "Shared/RmlModel_VideoSourceComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"
#include "StringUtils.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "TextureSourceComponent.h"
#include "TextureSourceSystem.h"
#include "TextureSourceSystemConfig.h"
#include "VideoSourceComponent.h"
#include "VideoSourceSystem.h"
#include "VideoSourceSystemConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectSources::RmlModel_ProjectSources()
	: m_videoSourceIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_selectedUSBVideoSourceModel(std::make_shared<RmlModel_USBVideoSourceComponent>())
	, m_selectedNetworkVideoSourceModel(std::make_shared<RmlModel_NetworkVideoSourceComponent>())
	, m_textureSourceIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_selectedClientVideoSourceModel(std::make_shared<RmlModel_ClientTextureSourceComponent>())
	, m_selectedSpoutVideoSourceModel(std::make_shared<RmlModel_SpoutTextureSourceComponent>())
{
}

bool RmlModel_ProjectSources::init(
	Rml::Context* rmlContext, 
	ProjectConfigPtr projectConfig,
	TextureSourceSystemPtr textureSourceSystem,
	VideoSourceSystemPtr videoSourceSystem)
{
	VideoSourceSystemConfigPtr videoSourceConfig = projectConfig->videoSourceSystemConfig;

	m_projectConfig = projectConfig;
	m_textureSourceSystem = textureSourceSystem;
	m_videoSourceSystem = videoSourceSystem;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "Sources");
	if (!constructor)
		return false;

	// Register component lists
	m_textureSourceIdList->init(
		constructor,
		videoSourceConfig,
		"texture_source_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			outComponentIdList = m_textureSourceSystem.lock()->getTextureSourceIdList();
		});
	m_textureSourceIdList->init(
		constructor, 
		videoSourceConfig,
		"video_source_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			outComponentIdList= m_videoSourceSystem.lock()->getVideoSourceIdList();
		});

	// Register Data Model Fields
	constructor.Bind("selected_video_source_id", &m_selectedVideoSourceId);
	constructor.Bind("selected_texture_source_id", &m_selectedTextureSourceId);

	// Register Selected Object Models
	m_selectedClientVideoSourceModel->init(rmlContext);
	m_selectedUSBVideoSourceModel->init(rmlContext);
	m_selectedNetworkVideoSourceModel->init(rmlContext);
	m_selectedSpoutVideoSourceModel->init(rmlContext);

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_usb_video_source", &RmlModel_ProjectSources::addNewUSBVideoSource, this);
	constructor.BindEventCallback("add_new_network_video_source", &RmlModel_ProjectSources::addNewNetworkVideoSource, this);
	constructor.BindEventCallback("remove_video_source", &RmlModel_ProjectSources::removeVideoSource, this);
	constructor.BindEventCallback("select_video_source_entry", &RmlModel_ProjectSources::selectVideoSourceEntry, this);
	constructor.BindEventCallback("add_new_client_texture_source", &RmlModel_ProjectSources::addNewClientTextureSource, this);
	constructor.BindEventCallback("add_new_spout_texture_source", &RmlModel_ProjectSources::addNewSpoutTextureSource, this);
	constructor.BindEventCallback("remove_texture_source", &RmlModel_ProjectSources::removeTextureSource, this);
	constructor.BindEventCallback("select_texture_source_entry", &RmlModel_ProjectSources::selectTextureSourceEntry, this);

	// Listen for video source config changes
	m_textureSourceIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectSources::videoSourceIdListChanged);
	m_textureSourceIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectSources::textureSourceIdListChanged);

	return true;
}

void RmlModel_ProjectSources::dispose()
{
	m_textureSourceIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectSources::videoSourceIdListChanged);
	m_textureSourceIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectSources::textureSourceIdListChanged);

	m_selectedClientVideoSourceModel->dispose();
	m_selectedUSBVideoSourceModel->dispose();
	m_selectedNetworkVideoSourceModel->dispose();
	m_selectedSpoutVideoSourceModel->dispose();

	RmlModel::dispose();
}

void RmlModel_ProjectSources::textureSourceIdListChanged(bool bOwnerChanged)
{
	MikanVideoSourceID selectedTextureSourceId = INVALID_MIKAN_ID;
	if (!m_textureSourceIdList->isEmpty() &&
		!m_textureSourceIdList->contains(m_selectedTextureSourceId))
	{
		selectedTextureSourceId = m_textureSourceIdList->getFirstValue();
	}

	setSelectedTextureSourceId(selectedTextureSourceId);
}

void RmlModel_ProjectSources::videoSourceIdListChanged(bool bOwnerChanged)
{
	MikanVideoSourceID selectedVideoSourceId = INVALID_MIKAN_ID;
	if (!m_textureSourceIdList->isEmpty() &&
		!m_textureSourceIdList->contains(m_selectedVideoSourceId))
	{
		selectedVideoSourceId = m_textureSourceIdList->getFirstValue();
	}

	setSelectedVideoSourceId(selectedVideoSourceId);
}

TextureSourceSystemPtr RmlModel_ProjectSources::getTextureSourceSystem()
{
	return m_textureSourceSystem.lock();
}

VideoSourceSystemPtr RmlModel_ProjectSources::getVideoSourceSystem()
{
	return m_videoSourceSystem.lock();
}

TextureSourceComponentPtr RmlModel_ProjectSources::getSelectedTextureSource()
{
	return getTextureSourceSystem()->getTextureSourceById((MikanTextureSourceID)m_selectedTextureSourceId);
}

VideoSourceComponentPtr RmlModel_ProjectSources::getSelectedVideoSource()
{
	return getVideoSourceSystem()->getVideoSourceById((MikanVideoSourceID)m_selectedVideoSourceId);
}

ClientTextureSourceComponentPtr RmlModel_ProjectSources::getSelectedClientTextureSource()
{
	return getTextureSourceSystem()->getClientTextureSourceSystem()->getClientTextureSourceById(
		(MikanTextureSourceID)m_selectedVideoSourceId);
}

SpoutTextureSourceComponentPtr RmlModel_ProjectSources::getSelectedSpoutTextureSource()
{
	return getTextureSourceSystem()->getSpoutTextureSourceSystem()->getSpoutTextureSourceById(
		(MikanTextureSourceID)m_selectedVideoSourceId);
}

USBVideoSourceComponentPtr RmlModel_ProjectSources::getSelectedUSBVideoSource()
{
	return getVideoSourceSystem()->getUSBVideoSourceSystem()->getUSBVideoSourceById(
		(MikanVideoSourceID)m_selectedVideoSourceId);
}

NetworkVideoSourceComponentPtr RmlModel_ProjectSources::getSelectedNetworkVideoSource()
{
	return getVideoSourceSystem()->getNetworkVideoSourceSystem()->getNetworkVideoSourceById(
		(MikanVideoSourceID)m_selectedVideoSourceId);
}

void RmlModel_ProjectSources::addNewClientTextureSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getTextureSourceSystem()->getClientTextureSourceSystem()->addNewClientTextureSource();
}

void RmlModel_ProjectSources::addNewSpoutTextureSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getTextureSourceSystem()->getSpoutTextureSourceSystem()->addNewSpoutTextureSource();
}

void RmlModel_ProjectSources::addNewUSBVideoSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getVideoSourceSystem()->getUSBVideoSourceSystem()->addNewUSBVideoSource();
}

void RmlModel_ProjectSources::addNewNetworkVideoSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getVideoSourceSystem()->getNetworkVideoSourceSystem()->addNewNetworkVideoSource();
}

void RmlModel_ProjectSources::removeVideoSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int videoSourceId = parameters[0].Get<int>();
	
	getVideoSourceSystem()->removeVideoSource((MikanVideoSourceID)videoSourceId);
}

void RmlModel_ProjectSources::selectVideoSourceEntry(
	Rml::DataModelHandle handle,
	Rml::Event& ev,
	const Rml::VariantList& parameters)
{
	const int selectedVideoSourceId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);
	
	setSelectedVideoSourceId(selectedVideoSourceId);
}

void RmlModel_ProjectSources::setSelectedVideoSourceId(MikanVideoSourceID videoSourceId)
{
	if (videoSourceId != m_selectedVideoSourceId)
	{
		m_selectedVideoSourceId = (int)videoSourceId;
		m_modelHandle.DirtyVariable("selected_video_source_id");

		VideoSourceSystemConfigPtr config = m_projectConfig.lock()->videoSourceSystemConfig;
		eVideoSourceType sourceType = config->getVideoSourceType(videoSourceId);

		// Reset all models first
		m_selectedUSBVideoSourceModel->setComponent(nullptr);
		m_selectedNetworkVideoSourceModel->setComponent(nullptr);

		// Set the appropriate model based on source type
		switch (sourceType)
		{
			case eVideoSourceType::usb:
				if (USBVideoSourceComponentPtr usbSource = getSelectedUSBVideoSource())
				{
					m_selectedUSBVideoSourceModel->setComponent(usbSource);
				}
				break;
			case eVideoSourceType::networked:
				if (NetworkVideoSourceComponentPtr networkSource = getSelectedNetworkVideoSource())
				{
					m_selectedNetworkVideoSourceModel->setComponent(networkSource);
				}
				break;
		}
	}
}

void RmlModel_ProjectSources::removeTextureSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int textureSourceId = parameters[0].Get<int>();

	getTextureSourceSystem()->removeTextureSource((MikanTextureSourceID)textureSourceId);
}

void RmlModel_ProjectSources::selectTextureSourceEntry(
	Rml::DataModelHandle handle,
	Rml::Event& ev,
	const Rml::VariantList& parameters)
{
	const int selectedTextureSourceId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

	setSelectedTextureSourceId(selectedTextureSourceId);
}

void RmlModel_ProjectSources::setSelectedTextureSourceId(MikanTextureSourceID textureSourceId)
{
	if (textureSourceId != m_selectedTextureSourceId)
	{
		m_selectedTextureSourceId = (int)textureSourceId;
		m_modelHandle.DirtyVariable("selected_texture_source_id");

		TextureSourceSystemConfigPtr config = m_projectConfig.lock()->textureSourceSystemConfig;
		eTextureSourceType sourceType = config->getTextureSourceType(textureSourceId);

		// Reset all models first
		m_selectedClientVideoSourceModel->setComponent(nullptr);
		m_selectedSpoutVideoSourceModel->setComponent(nullptr);

		// Set the appropriate model based on source type
		switch (sourceType)
		{
		case eTextureSourceType::client:
			if (ClientTextureSourceComponentPtr clientSource = getSelectedClientTextureSource())
			{
				m_selectedClientVideoSourceModel->setComponent(clientSource);
			}
			break;
		case eTextureSourceType::spout:
			if (SpoutTextureSourceComponentPtr spoutSource = getSelectedSpoutTextureSource())
			{
				m_selectedSpoutVideoSourceModel->setComponent(spoutSource);
			}
			break;
		}
	}
}