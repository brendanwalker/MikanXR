#include "ClientVideoSourceComponent.h"
#include "ClientVideoSourceSystem.h"
#include "RmlModel_ProjectSources.h"
#include "MikanCoreTypes.h"
#include "NetworkVideoSourceComponent.h"
#include "NetworkVideoSourceSystem.h"
#include "ProjectConfig.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "Shared/RmlModel_USBVideoSourceComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "SpoutVideoSourceComponent.h"
#include "SpoutVideoSourceSystem.h"
#include "StringUtils.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceComponent.h"
#include "VideoSourceSystem.h"
#include "VideoSourceSystemConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectSources::RmlModel_ProjectSources()
	: m_videoSourceIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_selectedClientVideoSourceModel(std::make_shared<RmlModel_PropertyInterface>())
	, m_selectedUSBVideoSourceModel(std::make_shared<RmlModel_USBVideoSourceComponent>())
	, m_selectedNetworkVideoSourceModel(std::make_shared<RmlModel_PropertyInterface>())
	, m_selectedSpoutVideoSourceModel(std::make_shared<RmlModel_PropertyInterface>())
{
}

bool RmlModel_ProjectSources::init(
	Rml::Context* rmlContext, 
	ProjectConfigPtr projectConfig,
	VideoSourceSystemPtr videoSourceSystem)
{
	VideoSourceSystemConfigPtr videoSourceConfig = projectConfig->videoSourceSystemConfig;

	m_projectConfig = projectConfig;
	m_videoSourceSystem = videoSourceSystem;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "video_sources");
	if (!constructor)
		return false;

	// Register component lists
	m_videoSourceIdList->init(
		constructor, 
		videoSourceConfig,
		"video_source_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			outComponentIdList= m_videoSourceSystem.lock()->getVideoSourceIdList();
		});

	// Register Data Model Fields
	constructor.Bind("selected_video_source_id", &m_selectedVideoSourceId);

	// Register Selected Object Models
	m_selectedClientVideoSourceModel->init<ClientVideoSourceComponent>(
		rmlContext,
		"client_video_source_definition");
	m_selectedUSBVideoSourceModel->init(rmlContext);
	m_selectedNetworkVideoSourceModel->init<NetworkVideoSourceComponent>(
		rmlContext,
		"network_video_source_definition");
	m_selectedSpoutVideoSourceModel->init<SpoutVideoSourceComponent>(
		rmlContext,
		"spout_video_source_definition");

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_client_video_source", &RmlModel_ProjectSources::addNewClientVideoSource, this);
	constructor.BindEventCallback("add_new_usb_video_source", &RmlModel_ProjectSources::addNewUSBVideoSource, this);
	constructor.BindEventCallback("add_new_network_video_source", &RmlModel_ProjectSources::addNewNetworkVideoSource, this);
	constructor.BindEventCallback("add_new_spout_video_source", &RmlModel_ProjectSources::addNewSpoutVideoSource, this);
	constructor.BindEventCallback("remove_video_source", &RmlModel_ProjectSources::removeVideoSource, this);
	constructor.BindEventCallback("select_video_source_entry", &RmlModel_ProjectSources::selectVideoSourceEntry, this);

	// Listen for video source config changes
	m_videoSourceIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectSources::videoSourceIdListChanged);

	return true;
}

void RmlModel_ProjectSources::dispose()
{
	m_videoSourceIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectSources::videoSourceIdListChanged);

	m_selectedClientVideoSourceModel->dispose();
	m_selectedUSBVideoSourceModel->dispose();
	m_selectedNetworkVideoSourceModel->dispose();
	m_selectedSpoutVideoSourceModel->dispose();

	RmlModel::dispose();
}

void RmlModel_ProjectSources::videoSourceIdListChanged(bool bOwnerChanged)
{
	MikanVideoSourceID selectedVideoSourceId = INVALID_MIKAN_ID;
	if (!m_videoSourceIdList->isEmpty() &&
		!m_videoSourceIdList->contains(m_selectedVideoSourceId))
	{
		selectedVideoSourceId = m_videoSourceIdList->getFirstValue();
	}

	setSelectedVideoSourceId(selectedVideoSourceId);
}

VideoSourceSystemPtr RmlModel_ProjectSources::getVideoSourceSystem()
{
	return m_videoSourceSystem.lock();
}

VideoSourceComponentPtr RmlModel_ProjectSources::getSelectedVideoSource()
{
	return getVideoSourceSystem()->getVideoSourceById((MikanVideoSourceID)m_selectedVideoSourceId);
}

ClientVideoSourceComponentPtr RmlModel_ProjectSources::getSelectedClientVideoSource()
{
	return getVideoSourceSystem()->getClientVideoSourceSystem()->getClientVideoSourceById(
		(MikanVideoSourceID)m_selectedVideoSourceId);
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

SpoutVideoSourceComponentPtr RmlModel_ProjectSources::getSelectedSpoutVideoSource()
{
	return getVideoSourceSystem()->getSpoutVideoSourceSystem()->getSpoutVideoSourceById(
		(MikanVideoSourceID)m_selectedVideoSourceId);
}

void RmlModel_ProjectSources::addNewClientVideoSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getVideoSourceSystem()->getClientVideoSourceSystem()->addNewClientVideoSource();
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

void RmlModel_ProjectSources::addNewSpoutVideoSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getVideoSourceSystem()->getSpoutVideoSourceSystem()->addNewSpoutVideoSource();
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
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const MikanVideoSourceID selectedVideoSourceId = (MikanVideoSourceID)parameters[0].Get<int>();
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
		m_selectedClientVideoSourceModel->setPropertyInterface(nullptr);
		m_selectedUSBVideoSourceModel->setComponent(nullptr);
		m_selectedNetworkVideoSourceModel->setPropertyInterface(nullptr);
		m_selectedSpoutVideoSourceModel->setPropertyInterface(nullptr);

		// Set the appropriate model based on source type
		switch (sourceType)
		{
			case eVideoSourceType::client:
				if (ClientVideoSourceComponentPtr clientSource = getSelectedClientVideoSource())
				{
					m_selectedClientVideoSourceModel->setPropertyInterface(clientSource, clientSource->getDefinition());
				}
				break;
			case eVideoSourceType::usb:
				if (USBVideoSourceComponentPtr usbSource = getSelectedUSBVideoSource())
				{
					m_selectedUSBVideoSourceModel->setComponent(usbSource);
				}
				break;
			case eVideoSourceType::networked:
				if (NetworkVideoSourceComponentPtr networkSource = getSelectedNetworkVideoSource())
				{
					m_selectedNetworkVideoSourceModel->setPropertyInterface(networkSource, networkSource->getDefinition());
				}
				break;
			case eVideoSourceType::spout:
				if (SpoutVideoSourceComponentPtr spoutSource = getSelectedSpoutVideoSource())
				{
					m_selectedSpoutVideoSourceModel->setPropertyInterface(spoutSource, spoutSource->getDefinition());
				}
				break;
		}
	}
}