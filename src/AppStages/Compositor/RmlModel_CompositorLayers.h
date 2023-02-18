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

struct RmlModel_CompositorLayer
{
	int layer_index;
	Rml::String material_name;
	Rml::Vector<RmlModel_LayerDataSourceMapping> color_texture_sources;
};

class RmlModel_CompositorLayers : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class GlFrameCompositor* compositor);
	virtual void dispose() override;

	SinglecastDelegate<void(const Rml::String& configName)> OnCompositorConfigChangedEvent;
	SinglecastDelegate<void(const Rml::String& clientSourceName)> OnScreenshotClientSourceEvent;
	SinglecastDelegate<void(const int layerIndex, const Rml::String& uniformName, const Rml::String& dataSourceName)> OnColorTextureMappingChangedEvent;

	void rebuild(const class GlFrameCompositor* compositor);

private:
	Rml::String m_currentConfigurationName;
	Rml::Vector<Rml::String> m_configurationNames;
	Rml::Vector<Rml::String> m_colorTextureSources;
	Rml::Vector<RmlModel_CompositorClient> m_compositorClients;
	Rml::Vector<RmlModel_CompositorLayer> m_compositorLayers;

	static bool s_bHasRegisteredTypes;
};
