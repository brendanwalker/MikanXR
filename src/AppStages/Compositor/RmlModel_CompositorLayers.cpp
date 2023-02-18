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

		// One time registration for layer data source struct.
		if (auto data_source_model_handle = constructor.RegisterStruct<RmlModel_LayerDataSourceMapping>())
		{
			data_source_model_handle.RegisterMember("uniform_name", &RmlModel_LayerDataSourceMapping::uniform_name);
			data_source_model_handle.RegisterMember("data_source_name", &RmlModel_LayerDataSourceMapping::data_source_name);
		}

		// One time registration for an array of layer data source mappings.
		constructor.RegisterArray<Rml::Vector<RmlModel_LayerDataSourceMapping>>();

		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorLayer>())
		{
			layer_model_handle.RegisterMember("layer_index", &RmlModel_CompositorLayer::layer_index);
			layer_model_handle.RegisterMember("material_name", &RmlModel_CompositorLayer::material_name);
			layer_model_handle.RegisterMember("color_texture_sources", &RmlModel_CompositorLayer::color_texture_sources);
		}

		// One time registration for an array of compositor layer.
		constructor.RegisterArray<decltype(m_compositorLayers)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("current_configuration", &m_currentConfigurationName);
	constructor.Bind("configuration_names", &m_configurationNames);
	constructor.Bind("color_texture_sources", &m_colorTextureSources);
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
	constructor.BindEventCallback(
		"set_color_texture_mapping",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int layer_index = (arguments.size() == 2 ? arguments[0].Get<int>() : -1);
			const std::string uniform_name = (arguments.size() == 2 ? arguments[1].Get<std::string>() : "");
			const std::string data_source_name = ev.GetParameter<Rml::String>("value", "");

			if (OnColorTextureMappingChangedEvent)
			{
				OnColorTextureMappingChangedEvent(layer_index, uniform_name, data_source_name);
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

	// Add color texture source names
	m_colorTextureSources.clear();
	for (auto it = compositor->getColorTextureSources().getMap().begin();
		 it != compositor->getColorTextureSources().getMap().end();
		 it++)
	{
		m_colorTextureSources.push_back(it->first);
	}
	m_modelHandle.DirtyVariable("color_texture_sources");

	// Add layers
	m_compositorLayers.clear();
	for (const auto& layer : compositor->getLayers())
	{
		const GlMaterial* layerMaterial= layer.layerMaterial;
		const CompositorLayerConfig* layerConfig= compositor->getCurrentPresetLayerConfig(layer.layerIndex);
		const std::string materialName= layerMaterial != nullptr ? layerMaterial->getName() : "INVALID";
		RmlModel_CompositorLayer uiLayer;
		uiLayer.layer_index= layer.layerIndex;
		uiLayer.material_name= materialName;

		if (layerConfig != nullptr)
		{
			for (auto it = layerConfig->shaderConfig.colorTextureSourceMap.begin();
				 it != layerConfig->shaderConfig.colorTextureSourceMap.end();
				 it++)
			{
				uiLayer.color_texture_sources.push_back({it->first, it->second});
			}
		}

		m_compositorLayers.push_back(uiLayer);
	}
	m_modelHandle.DirtyVariable("layers");

	// Add clients
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