#include "RmlModel_CompositorLayers.h"
#include "GlFrameCompositor.h"
#include "GlMaterial.h"
#include "GlProgram.h"
#include "ProfileConfig.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Event.h>

#include <vector>

bool RmlModel_CompositorLayers::s_bHasRegisteredTypes= false;

bool RmlModel_CompositorLayers::init(
	Rml::Context* rmlContext,
	const GlFrameCompositor* compositor,
	const ProfileConfig* profile)
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

		// One time registration for layer data source mapping struct.
		if (auto data_source_model_handle = constructor.RegisterStruct<RmlModel_LayerDataSourceMapping>())
		{
			data_source_model_handle.RegisterMember("uniform_name", &RmlModel_LayerDataSourceMapping::uniform_name);
			data_source_model_handle.RegisterMember("data_source_name", &RmlModel_LayerDataSourceMapping::data_source_name);
		}

		// One time registration for an array of layer data source mappings.
		constructor.RegisterArray<Rml::Vector<RmlModel_LayerDataSourceMapping>>();
		
		// One time registration for layer stencil flag struct.
		if (auto data_source_model_handle = constructor.RegisterStruct<RmlModel_LayerStencilFlag>())
		{
			data_source_model_handle.RegisterMember("stencil_id", &RmlModel_LayerStencilFlag::stencil_id);
			data_source_model_handle.RegisterMember("stencil_enabled", &RmlModel_LayerStencilFlag::stencil_enabled);
		}

		// One time registration for an array of layer stencil flags.
		constructor.RegisterArray<Rml::Vector<RmlModel_LayerStencilFlag>>();

		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorLayer>())
		{
			layer_model_handle.RegisterMember("layer_index", &RmlModel_CompositorLayer::layer_index);
			layer_model_handle.RegisterMember("material_name", &RmlModel_CompositorLayer::material_name);
			layer_model_handle.RegisterMember("vertical_flip", &RmlModel_CompositorLayer::vertical_flip);
			layer_model_handle.RegisterMember("blend_mode", &RmlModel_CompositorLayer::blend_mode);
			layer_model_handle.RegisterMember("float_mappings", &RmlModel_CompositorLayer::float_mappings);
			layer_model_handle.RegisterMember("float2_mappings", &RmlModel_CompositorLayer::float2_mappings);
			layer_model_handle.RegisterMember("float3_mappings", &RmlModel_CompositorLayer::float3_mappings);
			layer_model_handle.RegisterMember("float4_mappings", &RmlModel_CompositorLayer::float4_mappings);
			layer_model_handle.RegisterMember("mat4_mappings", &RmlModel_CompositorLayer::mat4_mappings);
			layer_model_handle.RegisterMember("color_texture_mappings", &RmlModel_CompositorLayer::color_texture_mappings);
			layer_model_handle.RegisterMember("quad_stencil_mode", &RmlModel_CompositorLayer::quad_stencil_mode);
			layer_model_handle.RegisterMember("quad_stencil_flags", &RmlModel_CompositorLayer::quad_stencil_flags);
			layer_model_handle.RegisterMember("invert_quads_when_camera_inside", &RmlModel_CompositorLayer::invert_quads_when_camera_inside);
			layer_model_handle.RegisterMember("box_stencil_mode", &RmlModel_CompositorLayer::box_stencil_mode);
			layer_model_handle.RegisterMember("box_stencil_flags", &RmlModel_CompositorLayer::box_stencil_flags);
			layer_model_handle.RegisterMember("model_stencil_mode", &RmlModel_CompositorLayer::model_stencil_mode);
			layer_model_handle.RegisterMember("model_stencil_flags", &RmlModel_CompositorLayer::model_stencil_flags);
		}

		// One time registration for an array of compositor layer.
		constructor.RegisterArray<decltype(m_compositorLayers)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("current_configuration", &m_currentConfigurationName);
	constructor.Bind("configuration_names", &m_configurationNames);
	constructor.Bind("material_names", &m_materialNames);
	constructor.Bind("blend_modes", &m_blendModes);
	constructor.Bind("stencil_modes", &m_stencilModes);
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
		"set_configuration",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string configurationName = ev.GetParameter<Rml::String>("value", "");

			if (OnCompositorConfigChangedEvent) 
			{
				OnCompositorConfigChangedEvent(configurationName);
			}
		});
	constructor.BindEventCallback(
		"update_material_name",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnMaterialNameChangeEvent)
			{
				const int layer_index = (arguments.size() == 1 ? arguments[0].Get<int>() : -1);
				const std::string material_name = ev.GetParameter<Rml::String>("value", "");

				OnMaterialNameChangeEvent(layer_index, material_name);
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
		"update_blend_mode",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnBlendModeChangeEvent)
			{
				const int layer_index = (arguments.size() == 1 ? arguments[0].Get<int>() : -1);

				if (layer_index != -1)
				{
					const std::string blendModeString = ev.GetParameter<Rml::String>("value", "");
					const eCompositorBlendMode blendMode =
						StringUtils::FindEnumValue<eCompositorBlendMode>(
							blendModeString,
							k_compositorBlendModeStrings);

					OnBlendModeChangeEvent(layer_index, blendMode);
				}
			}
		});
	constructor.BindEventCallback(
		"toggle_invert_quads",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnInvertQuadsFlagChangeEvent)
			{
				const int layer_index = (arguments.size() == 1 ? arguments[0].Get<int>() : -1);
				const std::string value = ev.GetParameter<Rml::String>("value", "");
				const bool bIsChecked = !value.empty();

				if (layer_index != -1)
				{
					OnInvertQuadsFlagChangeEvent(layer_index, bIsChecked);
				}
			}
		});
	constructor.BindEventCallback(
		"update_quad_stencil_mode",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			invokeStencilModeChangeDelegate(model, ev, arguments, OnQuadStencilModeChangeEvent);
		});
	constructor.BindEventCallback(
		"update_box_stencil_mode",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			invokeStencilModeChangeDelegate(model, ev, arguments, OnBoxStencilModeChangeEvent);
		});
	constructor.BindEventCallback(
		"update_model_stencil_mode",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			invokeStencilModeChangeDelegate(model, ev, arguments, OnModelStencilModeChangeEvent);
		});
	constructor.BindEventCallback(
		"toggle_stencil_enabled",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int layer_index = (arguments.size() == 2 ? arguments[0].Get<int>() : -1);
			const MikanStencilID stencil_id = (arguments.size() == 2 ? arguments[1].Get<int>() : INVALID_MIKAN_ID);
			const std::string value = ev.GetParameter<Rml::String>("value", "");
			const bool bIsEnabled = !value.empty();

			if (layer_index != -1 && stencil_id != INVALID_MIKAN_ID)
			{
				if (bIsEnabled)
				{
					if (OnStencilRefAddedEvent)
						OnStencilRefAddedEvent(layer_index, stencil_id);
				}
				else
				{
					if (OnStencilRefRemovedEvent)
						OnStencilRefRemovedEvent(layer_index, stencil_id);
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
	rebuild(compositor, profile);

	return true;
}

void RmlModel_CompositorLayers::invokeStencilModeChangeDelegate(
	Rml::DataModelHandle model,
	Rml::Event& ev,
	const Rml::VariantList& arguments,
	StencilModeChangedDelegate& stencilModeChangedDelegate)
{
	if (stencilModeChangedDelegate)
	{
		const int layer_index = (arguments.size() == 1 ? arguments[0].Get<int>() : -1);

		if (layer_index != -1)
		{
			const std::string stencilModeString = ev.GetParameter<Rml::String>("value", "");
			const eCompositorStencilMode stencilMode =
				StringUtils::FindEnumValue<eCompositorStencilMode>(
					stencilModeString,
					k_compositorStencilModeStrings);

			stencilModeChangedDelegate(layer_index, stencilMode);
		}
	}
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
	RmlModel::dispose();
}

void RmlModel_CompositorLayers::rebuild(
	const GlFrameCompositor* compositor,
	const ProfileConfig* profile)
{
	m_currentConfigurationName= compositor->getCurrentPresetName();
	m_configurationNames= compositor->getPresetNames();
	m_materialNames= compositor->getAllCompositorShaderNames();

	// Fill in blend mode strings
	size_t blendModeCount = (size_t)eCompositorBlendMode::COUNT;
	m_blendModes= Rml::Vector<Rml::String>(blendModeCount);
	std::copy(k_compositorBlendModeStrings, 
			  k_compositorBlendModeStrings+blendModeCount, 
			  m_blendModes.begin());

	// Fill in stencil mode strings
	size_t stencilModeCount = (size_t)eCompositorStencilMode::COUNT;
	m_stencilModes = Rml::Vector<Rml::String>(stencilModeCount);
	std::copy(k_compositorStencilModeStrings, 
			  k_compositorStencilModeStrings + stencilModeCount, 
			  m_stencilModes.begin());	

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
		const GlMaterialPtr layerMaterial= layer.layerMaterial;
		const CompositorLayerConfig* layerConfig= compositor->getCurrentPresetLayerConfig(layer.layerIndex);
		const std::string materialName= layerMaterial != nullptr ? layerMaterial->getName() : "INVALID";
		
		RmlModel_CompositorLayer uiLayer;
		uiLayer.layer_index= layer.layerIndex;
		uiLayer.material_name= materialName;
		uiLayer.blend_mode= k_compositorBlendModeStrings[(int)layerConfig->blendMode];
		uiLayer.vertical_flip = layerConfig->verticalFlip;

		// Add quad stencil properties
		uiLayer.quad_stencil_mode = k_compositorStencilModeStrings[(int)layerConfig->quadStencilConfig.stencilMode];
		uiLayer.invert_quads_when_camera_inside = layerConfig->quadStencilConfig.bInvertWhenCameraInside;
		uiLayer.quad_stencil_flags.clear();
		for (const auto& stencil : profile->quadStencilList)
		{
			const auto& enabledStencilIds= layerConfig->quadStencilConfig.quadStencilIds;

			RmlModel_LayerStencilFlag stencilFlag;
			stencilFlag.stencil_id= stencil.stencil_id;
			stencilFlag.stencil_enabled= 
				std::find(
					enabledStencilIds.begin(),
					enabledStencilIds.end(),
					stencil.stencil_id) != enabledStencilIds.end();

			uiLayer.quad_stencil_flags.push_back(stencilFlag);
		}

		// Add box stencil IDs
		uiLayer.box_stencil_mode = k_compositorStencilModeStrings[(int)layerConfig->boxStencilConfig.stencilMode];
		uiLayer.box_stencil_flags.clear();
		for (const auto& stencil : profile->boxStencilList)
		{
			const auto& enabledStencilIds= layerConfig->boxStencilConfig.boxStencilIds;

			RmlModel_LayerStencilFlag stencilFlag;
			stencilFlag.stencil_id = stencil.stencil_id;
			stencilFlag.stencil_enabled =
				std::find(
					enabledStencilIds.begin(),
					enabledStencilIds.end(),
					stencil.stencil_id) != enabledStencilIds.end();

			uiLayer.box_stencil_flags.push_back(stencilFlag);
		}

		// Add model stencil IDs
		uiLayer.model_stencil_mode = k_compositorStencilModeStrings[(int)layerConfig->modelStencilConfig.stencilMode];
		uiLayer.model_stencil_flags.clear();
		for (const auto& stencil : profile->modelStencilList)
		{
			const auto& enabledStencilIds= layerConfig->modelStencilConfig.modelStencilIds;
			
			RmlModel_LayerStencilFlag stencilFlag;
			stencilFlag.stencil_id = stencil.modelInfo.stencil_id;
			stencilFlag.stencil_enabled =
				std::find(
					enabledStencilIds.begin(),
					enabledStencilIds.end(),
					stencil.modelInfo.stencil_id) != enabledStencilIds.end();

			uiLayer.model_stencil_flags.push_back(stencilFlag);
		}

		if (layerMaterial != nullptr)
		{
			auto copyMaterialBindings= [](
				const std::vector<std::string>& materialUniformNames,
				const std::map<std::string, std::string>* configSourceMappings,
				Rml::Vector<RmlModel_LayerDataSourceMapping>& uiSourceMappings)
			{
				// For each material uniform, use the mapping found in the config
				// to find the name of the source in the compositor it pulls data from
				for (const std::string& uniformName : materialUniformNames)
				{
					if (configSourceMappings != nullptr)
					{
						auto it = configSourceMappings->find(uniformName);

						if (it != configSourceMappings->end())
						{
							uiSourceMappings.push_back({uniformName, it->second});
							continue;
						}
					}

					// Fallback to empty mapping if no corresponding mapping found in the config
					uiSourceMappings.push_back({uniformName, "empty"});
				}
			};

			const CompositorLayerShaderConfig* shaderConfig= 
				(layerConfig != nullptr) ? &layerConfig->shaderConfig : nullptr;
			GlProgramConstPtr program= layerMaterial->getProgram();

			copyMaterialBindings(
				program->getUniformNamesOfDataType(eUniformDataType::datatype_float),
				(shaderConfig != nullptr) ? &shaderConfig->floatSourceMap : nullptr,
				uiLayer.float_mappings);
			copyMaterialBindings(
				program->getUniformNamesOfDataType(eUniformDataType::datatype_float2),
				(shaderConfig != nullptr) ? &shaderConfig->float2SourceMap : nullptr,
				uiLayer.float2_mappings);
			copyMaterialBindings(
				program->getUniformNamesOfDataType(eUniformDataType::datatype_float3),
				(shaderConfig != nullptr) ? &shaderConfig->float3SourceMap : nullptr,
				uiLayer.float3_mappings);
			copyMaterialBindings(
				program->getUniformNamesOfDataType(eUniformDataType::datatype_float4),
				(shaderConfig != nullptr) ? &shaderConfig->float4SourceMap : nullptr,
				uiLayer.float4_mappings);
			copyMaterialBindings(
				program->getUniformNamesOfDataType(eUniformDataType::datatype_mat4),
				(shaderConfig != nullptr) ? &shaderConfig->mat4SourceMap : nullptr,
				uiLayer.mat4_mappings);
			copyMaterialBindings(
				program->getUniformNamesOfDataType(eUniformDataType::datatype_texture),
				(shaderConfig != nullptr) ? &shaderConfig->colorTextureSourceMap : nullptr, 
				uiLayer.color_texture_mappings);
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