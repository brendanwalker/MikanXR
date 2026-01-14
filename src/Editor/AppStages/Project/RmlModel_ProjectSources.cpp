#include "ClientTextureSourceComponent.h"
#include "ClientTextureSourceSystem.h"
#include "RmlModel_ProjectSources.h"
#include "MikanCoreTypes.h"
#include "NetworkVideoSourceComponent.h"
#include "NetworkVideoSourceSystem.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectRmlModelContext.h"
#include "Shared/RmlModel_ClientTextureSourceComponent.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlModel_NetworkVideoSourceComponent.h"
#include "Shared/RmlModel_SpoutTextureSourceComponent.h"
#include "Shared/RmlModel_USBVideoSourceComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"
#include "StringUtils.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "TextureSourceComponent.h"
#include "TextureSourceQueries.h"
#include "VideoSourceComponent.h"
#include "VideoSourceQueries.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectSources::RmlModel_ProjectSources()
	: m_videoSourceIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_textureSourceIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{
}

bool RmlModel_ProjectSources::init(ProjectRmlModelContext* context)
{
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	Rml::Context* rmlContext = ownerAppStage->getRmlContext();

	m_projectRmlModelContext = context;
	m_projectManager = ownerAppStage->getProjectManager();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "Sources");
	if (!constructor)
		return false;

	// Register component lists
	m_textureSourceIdList->init(
		constructor,
		CommonConfigPtr(), // No single owner config since this is a virtual list
		"texture_source_ids", // virtual list since this is a combination of multiple source types
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			outComponentIdList = TextureSourceQueries::getTextureSourceIdList(m_projectManager.lock());
		},
		[this](const ConfigPropertyChangeSet& changedPropertySet) {
			return
				changedPropertySet.hasPropertyName(ClientTextureSourceSystemDefinition::k_componentIdListPropertyId) ||
				changedPropertySet.hasPropertyName(SpoutTextureSourceSystemDefinition::k_componentIdListPropertyId);
		});
	m_videoSourceIdList->init(
		constructor,
		CommonConfigPtr(), // No single owner config since this is a virtual list
		"video_source_ids", // virtual list since this is a combination of multiple source types
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			outComponentIdList= VideoSourceQueries::getVideoSourceIdList(m_projectManager.lock());
		},
		[this](const ConfigPropertyChangeSet& changedPropertySet) {
			return
				changedPropertySet.hasPropertyName(USBVideoSourceSystemDefinition::k_componentIdListPropertyId) ||
				changedPropertySet.hasPropertyName(NetworkVideoSourceSystemDefinition::k_componentIdListPropertyId);
		});

	// Register Data Model Fields
	constructor.Bind("selected_video_source_id", &m_selectedVideoSourceId);
	constructor.Bind("selected_texture_source_id", &m_selectedTextureSourceId);

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

	// Defer the selection update to post view update after element list refreshes
	addModelUpdateCallback([this, selectedTextureSourceId]() {
		setSelectedTextureSourceId(selectedTextureSourceId);
	});
}

void RmlModel_ProjectSources::videoSourceIdListChanged(bool bOwnerChanged)
{
	MikanVideoSourceID selectedVideoSourceId = INVALID_MIKAN_ID;
	if (!m_textureSourceIdList->isEmpty() &&
		!m_textureSourceIdList->contains(m_selectedVideoSourceId))
	{
		selectedVideoSourceId = m_textureSourceIdList->getFirstValue();
	}

	// Defer the selection update to post view update after element list refreshes
	addModelUpdateCallback([this, selectedVideoSourceId]() {
		setSelectedVideoSourceId(selectedVideoSourceId);
	});
}

TextureSourceComponentPtr RmlModel_ProjectSources::getSelectedTextureSource()
{
	return TextureSourceQueries::getTextureSourceById(
		m_projectManager.lock(),
		(MikanTextureSourceID)m_selectedTextureSourceId);
}

VideoSourceComponentPtr RmlModel_ProjectSources::getSelectedVideoSource()
{
	return VideoSourceQueries::getVideoSourceById(
		m_projectManager.lock(),
		(MikanVideoSourceID)m_selectedVideoSourceId);
}

ClientTextureSourceComponentPtr RmlModel_ProjectSources::getSelectedClientTextureSource()
{
	auto projectManager = m_projectManager.lock();
	auto clientTextureSourceSystem = projectManager->getSystemOfType<ClientTextureSourceSystem>();
	return clientTextureSourceSystem->getTypedComponentById(
		(MikanTextureSourceID)m_selectedTextureSourceId);
}

SpoutTextureSourceComponentPtr RmlModel_ProjectSources::getSelectedSpoutTextureSource()
{
	auto projectManager = m_projectManager.lock();
	auto spoutTextureSourceSystem = projectManager->getSystemOfType<SpoutTextureSourceSystem>();
	return spoutTextureSourceSystem->getTypedComponentById(
		(MikanTextureSourceID)m_selectedTextureSourceId);
}

USBVideoSourceComponentPtr RmlModel_ProjectSources::getSelectedUSBVideoSource()
{
	auto projectManager = m_projectManager.lock();
	auto usbVideoSourceSystem = projectManager->getSystemOfType<USBVideoSourceSystem>();
	return usbVideoSourceSystem->getTypedComponentById((MikanVideoSourceID)m_selectedVideoSourceId);
}

NetworkVideoSourceComponentPtr RmlModel_ProjectSources::getSelectedNetworkVideoSource()
{
	auto projectManager = m_projectManager.lock();
	auto networkVideoSourceSystem = projectManager->getSystemOfType<NetworkVideoSourceSystem>();
	return networkVideoSourceSystem->getTypedComponentById((MikanVideoSourceID)m_selectedVideoSourceId);
}

void RmlModel_ProjectSources::addNewClientTextureSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	auto projectManager = m_projectManager.lock();
	auto clientTextureSourceSystem = projectManager->getSystemOfType<ClientTextureSourceSystem>();
	clientTextureSourceSystem->addNewObject();
}

void RmlModel_ProjectSources::addNewSpoutTextureSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	auto projectManager = m_projectManager.lock();
	auto spoutTextureSourceSystem = projectManager->getSystemOfType<SpoutTextureSourceSystem>();
	spoutTextureSourceSystem->addNewObject();
}

void RmlModel_ProjectSources::addNewUSBVideoSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	auto projectManager = m_projectManager.lock();
	auto usbVideoSourceSystem = projectManager->getSystemOfType<USBVideoSourceSystem>();
	usbVideoSourceSystem->addNewUSBVideoSource();  // Keep custom method for USB device selection
}

void RmlModel_ProjectSources::addNewNetworkVideoSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	auto projectManager = m_projectManager.lock();
	auto networkVideoSourceSystem = projectManager->getSystemOfType<NetworkVideoSourceSystem>();
	networkVideoSourceSystem->addNewObject();
}

void RmlModel_ProjectSources::removeVideoSource(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	VideoSourceQueries::removeVideoSource(
		m_projectManager.lock(),
		(MikanVideoSourceID)m_selectedVideoSourceId);
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

		eVideoSourceType sourceType =
			VideoSourceQueries::getVideoSourceType(m_projectManager.lock(), videoSourceId);

		// Reset all models first
		m_projectRmlModelContext->getUSBVideoSourceModel()->setComponent(nullptr);
		m_projectRmlModelContext->getNetworkVideoSourceModel()->setComponent(nullptr);

		// Set the appropriate model based on source type
		switch (sourceType)
		{
			case eVideoSourceType::usb:
				if (USBVideoSourceComponentPtr usbSource = getSelectedUSBVideoSource())
				{
					m_projectRmlModelContext->getUSBVideoSourceModel()->setComponent(usbSource);
				}
				break;
			case eVideoSourceType::networked:
				if (NetworkVideoSourceComponentPtr networkSource = getSelectedNetworkVideoSource())
				{
					m_projectRmlModelContext->getNetworkVideoSourceModel()->setComponent(networkSource);
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
	TextureSourceQueries::removeTextureSource(
		m_projectManager.lock(),
		(MikanTextureSourceID)m_selectedTextureSourceId);
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

		eTextureSourceType sourceType =
			TextureSourceQueries::getTextureSourceType(m_projectManager.lock(), textureSourceId);

		// Reset all models first
		m_projectRmlModelContext->getClientTextureSourceModel()->setComponent(nullptr);
		m_projectRmlModelContext->getSpoutTextureSourceModel()->setComponent(nullptr);

		// Set the appropriate model based on source type
		switch (sourceType)
		{
		case eTextureSourceType::client:
			if (ClientTextureSourceComponentPtr clientSource = getSelectedClientTextureSource())
			{
				m_projectRmlModelContext->getClientTextureSourceModel()->setComponent(clientSource);
			}
			break;
		case eTextureSourceType::spout:
			if (SpoutTextureSourceComponentPtr spoutSource = getSelectedSpoutTextureSource())
			{
				m_projectRmlModelContext->getSpoutTextureSourceModel()->setComponent(spoutSource);
			}
			break;
		}
	}
}