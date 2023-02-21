#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorClient
{
	Rml::String client_id;
	Rml::String app_name;
};

struct RmlModel_LayerDataSourceMapping
{
	Rml::String uniform_name;
	Rml::String data_source_name;
};

struct RmlModel_LayerStencilFlag
{
	int stencil_id;
	bool stencil_enabled;
};

struct RmlModel_CompositorLayer
{
	int layer_index;
	bool vertical_flip;
	Rml::String material_name;
	Rml::String blend_mode;
	Rml::Vector<RmlModel_LayerDataSourceMapping> float_mappings;
	Rml::Vector<RmlModel_LayerDataSourceMapping> float2_mappings;
	Rml::Vector<RmlModel_LayerDataSourceMapping> float3_mappings;
	Rml::Vector<RmlModel_LayerDataSourceMapping> float4_mappings;
	Rml::Vector<RmlModel_LayerDataSourceMapping> mat4_mappings;
	Rml::Vector<RmlModel_LayerDataSourceMapping> color_texture_mappings;
	Rml::String quad_stencil_mode;
	bool invert_quads_when_camera_inside;
	Rml::Vector<RmlModel_LayerStencilFlag> quad_stencil_flags;
	Rml::String box_stencil_mode;
	Rml::Vector<RmlModel_LayerStencilFlag> box_stencil_flags;
	Rml::String model_stencil_mode;
	Rml::Vector<RmlModel_LayerStencilFlag> model_stencil_flags;
};

class RmlModel_CompositorLayers : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext, 
		const class GlFrameCompositor* compositor,
		const class ProfileConfig* profile);
	virtual void dispose() override;

	SinglecastDelegate<void(const Rml::String& configName)> OnCompositorConfigChangedEvent;
	SinglecastDelegate<void(const Rml::String& clientSourceName)> OnScreenshotClientSourceEvent;
	SinglecastDelegate<void(const int layerIndex, const Rml::String& materialName)> OnMaterialNameChangeEvent;
	SinglecastDelegate<void(const int layerIndex, bool bFlipFlag)> OnVerticalFlipChangeEvent;
	SinglecastDelegate<void(const int layerIndex, eCompositorBlendMode blendMode)> OnBlendModeChangeEvent;
	SinglecastDelegate<void(const int layerIndex, bool bInvertFlag)> OnInvertQuadsFlagChangeEvent;
	
	using StencilModeChangedDelegate = SinglecastDelegate<void(const int layerIndex, eCompositorStencilMode stencilMode)>;
	StencilModeChangedDelegate OnQuadStencilModeChangeEvent;
	StencilModeChangedDelegate OnBoxStencilModeChangeEvent;
	StencilModeChangedDelegate OnModelStencilModeChangeEvent;
	void invokeStencilModeChangeDelegate(
		Rml::DataModelHandle model,
		Rml::Event& ev,
		const Rml::VariantList& arguments,
		StencilModeChangedDelegate& mappingChangedDelegate);

	using StencilRefChangedDelegate = SinglecastDelegate<void(const int layerIndex, int stencilId)>;
	StencilRefChangedDelegate OnStencilRefAddedEvent;
	StencilRefChangedDelegate OnStencilRefRemovedEvent;

	using MappingChangedDelegate = SinglecastDelegate<void(const int layerIndex, const Rml::String& uniformName, const Rml::String& dataSourceName)>;
	MappingChangedDelegate OnFloatMappingChangedEvent;
	MappingChangedDelegate OnFloat2MappingChangedEvent;
	MappingChangedDelegate OnFloat3MappingChangedEvent;
	MappingChangedDelegate OnFloat4MappingChangedEvent;
	MappingChangedDelegate OnMat4MappingChangedEvent;
	MappingChangedDelegate OnColorTextureMappingChangedEvent;
	void invokeMappingChangeDelegate(
		Rml::DataModelHandle model,
		Rml::Event& ev,
		const Rml::VariantList& arguments,
		MappingChangedDelegate& mappingChangedDelegate);

	void rebuild(const class GlFrameCompositor* compositor, const class ProfileConfig* profile);

private:
	Rml::String m_currentConfigurationName;
	Rml::Vector<Rml::String> m_configurationNames;
	Rml::Vector<Rml::String> m_materialNames;
	Rml::Vector<Rml::String> m_blendModes;
	Rml::Vector<Rml::String> m_stencilModes;
	Rml::Vector<RmlModel_CompositorClient> m_compositorClients;
	Rml::Vector<RmlModel_CompositorLayer> m_compositorLayers;
	Rml::Vector<Rml::String> m_floatSources;
	Rml::Vector<Rml::String> m_float2Sources;
	Rml::Vector<Rml::String> m_float3Sources;
	Rml::Vector<Rml::String> m_float4Sources;
	Rml::Vector<Rml::String> m_mat4Sources;
	Rml::Vector<Rml::String> m_colorTextureSources;

	static bool s_bHasRegisteredTypes;
};
