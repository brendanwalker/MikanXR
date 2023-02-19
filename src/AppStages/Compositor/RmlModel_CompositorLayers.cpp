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
			layer_model_handle.RegisterMember("vertical_flip", &RmlModel_CompositorLayer::vertical_flip);
			layer_model_handle.RegisterMember("float_mappings", &RmlModel_CompositorLayer::float_mappings);
			layer_model_handle.RegisterMember("float2_mappings", &RmlModel_CompositorLayer::float2_mappings);
			layer_model_handle.RegisterMember("float3_mappings", &RmlModel_CompositorLayer::float3_mappings);
			layer_model_handle.RegisterMember("float4_mappings", &RmlModel_CompositorLayer::float4_mappings);
			layer_model_handle.RegisterMember("mat4_mappings", &RmlModel_CompositorLayer::mat4_mappings);
			layer_model_handle.RegisterMember("color_texture_mappings", &RmlModel_CompositorLayer::color_texture_mappings);
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
	constructor.Bind("float_sources", &m_floatSources);
	constructor.Bind("float2_sources", &m_float2Sources);
	constructor.Bind("float3_sources", &m_float3Sources);
	constructor.Bind("float4_sources", &m_float4Sources);
	constructor.Bind("mat4_sources", &m_mat4Sources);
	constructor.Bind("color_texture_sources", &m_colorTextureSources);

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
		"update_vertical_flip",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnVerticalFlipChangeEvent)
			{
				const int layer_index = (arguments.size() == 1 ? arguments[0].Get<int>() : -1);
				const std::string value = ev.GetParameter<Rml::String>("value", "");
				const bool bIsChecked= !value.empty();

				if (layer_index != -1)
				{
					OnVerticalFlipChangeEvent(layer_index, bIsChecked);
				}
			}
		});
	constructor.BindEventCallback(
		"update_float_mapping",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			invokeMappingChangeDelegate(model, ev, arguments, OnFloatMappingChangedEvent);
		});
	constructor.BindEventCallback(
		"update_float2_mapping",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			invokeMappingChangeDelegate(model, ev, arguments, OnFloat2MappingChangedEvent);
		});
	constructor.BindEventCallback(
		"update_float3_mapping",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			invokeMappingChangeDelegate(model, ev, arguments, OnFloat3MappingChangedEvent);
		});
	constructor.BindEventCallback(
		"update_float4_mapping",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			invokeMappingChangeDelegate(model, ev, arguments, OnFloat4MappingChangedEvent);
		});
	constructor.BindEventCallback(
		"update_mat4_mapping",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			invokeMappingChangeDelegate(model, ev, arguments, OnMat4MappingChangedEvent);
		});
	constructor.BindEventCallback(
		"update_color_texture_mapping",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			invokeMappingChangeDelegate(model, ev, arguments, OnColorTextureMappingChangedEvent);
		});

	// Set initial values for data model
	rebuild(compositor);

	return true;
}

void RmlModel_CompositorLayers::invokeMappingChangeDelegate(
	Rml::DataModelHandle model,
	Rml::Event& ev,
	const Rml::VariantList& arguments,
	MappingChangedDelegate& mappingChangedDelegate)
{
	if (mappingChangedDelegate)
	{
		const int layer_index = (arguments.size() == 2 ? arguments[0].Get<int>() : -1);
		const std::string uniform_name = (arguments.size() == 2 ? arguments[1].Get<std::string>() : "");
		const std::string data_source_name = ev.GetParameter<Rml::String>("value", "");

		mappingChangedDelegate(layer_index, uniform_name, data_source_name);
	}
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
	
	// Add float data source names
	m_floatSources.clear();
	for (auto it = compositor->getFloatSources().getMap().begin();
		 it != compositor->getFloatSources().getMap().end();
		 it++)
	{
		m_floatSources.push_back(it->first);
	}

	// Add float2 data source names
	m_float2Sources.clear();
	for (auto it = compositor->getFloat2Sources().getMap().begin();
		 it != compositor->getFloat2Sources().getMap().end();
		 it++)
	{
		m_float2Sources.push_back(it->first);
	}

	// Add float3 data source names
	m_float3Sources.clear();
	for (auto it = compositor->getFloat3Sources().getMap().begin();
		 it != compositor->getFloat3Sources().getMap().end();
		 it++)
	{
		m_float3Sources.push_back(it->first);
	}

	// Add float4 data source names
	m_float4Sources.clear();
	for (auto it = compositor->getFloat4Sources().getMap().begin();
		 it != compositor->getFloat4Sources().getMap().end();
		 it++)
	{
		m_float4Sources.push_back(it->first);
	}

	// Add mat4 data source names
	m_mat4Sources.clear();
	for (auto it = compositor->getMat4Sources().getMap().begin();
		 it != compositor->getMat4Sources().getMap().end();
		 it++)
	{
		m_mat4Sources.push_back(it->first);
	}

	// Add color texture data source names
	m_colorTextureSources.clear();
	for (auto it = compositor->getColorTextureSources().getMap().begin();
		 it != compositor->getColorTextureSources().getMap().end();
		 it++)
	{
		m_colorTextureSources.push_back(it->first);
	}

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
			auto copyMaterialSources= [](
				const std::map<std::string, std::string>& configSourceMappings,
				Rml::Vector<RmlModel_LayerDataSourceMapping>& uiSourceMappings)
			{
				for (auto it = configSourceMappings.begin(); it != configSourceMappings.end(); it++)
				{
					uiSourceMappings.push_back({it->first, it->second});
				}
			};

			const CompositorLayerShaderConfig& shaderConfig= layerConfig->shaderConfig;
			copyMaterialSources(shaderConfig.floatSourceMap, uiLayer.float_mappings);
			copyMaterialSources(shaderConfig.float2SourceMap, uiLayer.float2_mappings);
			copyMaterialSources(shaderConfig.float3SourceMap, uiLayer.float3_mappings);
			copyMaterialSources(shaderConfig.float4SourceMap, uiLayer.float4_mappings);
			copyMaterialSources(shaderConfig.mat4SourceMap, uiLayer.mat4_mappings);
			copyMaterialSources(shaderConfig.colorTextureSourceMap, uiLayer.color_texture_mappings);

			uiLayer.vertical_flip = layerConfig->verticalFlip;
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