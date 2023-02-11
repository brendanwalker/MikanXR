#include "RmlModel_CompositorLayers.h"
#include "GlFrameCompositor.h"
#include "GlMaterial.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Event.h>

bool RmlModel_CompositorLayers::s_bHasRegisteredTypes= false;

bool RmlModel_CompositorLayers::init(
	Rml::Context* rmlContext,
	const GlFrameCompositor* compositor)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_layers");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor client struct.
		if (auto client_model_handle = constructor.RegisterStruct<RmlModel_CompositorClient>())
		{
			client_model_handle.RegisterMember("client_id", &RmlModel_CompositorClient::client_id);
			client_model_handle.RegisterMember("app_name", &RmlModel_CompositorClient::app_name);
		}

		// One time registration for an array of compositor clients.
		constructor.RegisterArray<decltype(m_compositorClients)>();

		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorLayer>())
		{
			layer_model_handle.RegisterMember("layer_index", &RmlModel_CompositorLayer::layer_index);
			layer_model_handle.RegisterMember("material_name", &RmlModel_CompositorLayer::material_name);
		}

		// One time registration for an array of compositor layer.
		constructor.RegisterArray<decltype(m_compositorLayers)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("current_configuration", &m_currentConfigurationName);
	constructor.Bind("configuration_names", &m_configurationNames);
	constructor.Bind("clients", &m_compositorClients);
	constructor.Bind("layers", &m_compositorLayers);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"screenshot",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int listIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnScreenshotClientSourceEvent && listIndex >= 0)
			{
				OnScreenshotClientSourceEvent(m_compositorClients[listIndex].client_id);
			}
		});
	constructor.BindEventCallback(
		"set_configuration",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string configurationName = ev.GetParameter<Rml::String>("value", "");

			if (OnCompositorConfigChangedEvent) 
			{
				OnCompositorConfigChangedEvent(configurationName);
			}
		});

	// Set initial values for data model
	rebuild(compositor);

	return true;
}

void RmlModel_CompositorLayers::dispose()
{
	OnCompositorConfigChangedEvent.Clear();
	OnScreenshotClientSourceEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorLayers::rebuild(
	const GlFrameCompositor* compositor)
{
	m_currentConfigurationName= compositor->getCurrentPresetName();
	m_configurationNames= compositor->getPresetNames();

	m_compositorLayers.clear();
	for (const auto& layer : compositor->getLayers())
	{
		const std::string materialName= layer.layerMaterial != nullptr ? layer.layerMaterial->getName() : "INVALID";
		const RmlModel_CompositorLayer uiLayer= { layer.layerIndex, materialName};

		m_compositorLayers.push_back(uiLayer);
	}
	m_modelHandle.DirtyVariable("layers");

	const auto& clientSources = compositor->getClientSources();
	m_compositorClients.clear();
	for (auto it = clientSources.getMap().begin(); it != clientSources.getMap().end(); it++)
	{
		const GlFrameCompositor::ClientSource* clientSource= it->second;
		const RmlModel_CompositorClient uiClient = {clientSource->clientId, clientSource->clientInfo.applicationName };

		m_compositorClients.push_back(uiClient);
	}
	m_modelHandle.DirtyVariable("clients");
}