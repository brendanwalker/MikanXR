#include "RmlModel_CompositorSources.h"
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

bool RmlModel_CompositorSources::s_bHasRegisteredTypes= false;

bool RmlModel_CompositorSources::init(
	Rml::Context* rmlContext,
	const GlFrameCompositor* compositor)
{
	m_compositor= compositor;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_sources");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for layer data source mapping struct.
		if (auto data_source_model_handle = constructor.RegisterStruct<RmlModel_SourceMappingValue>())
		{
			data_source_model_handle.RegisterMember("source_name", &RmlModel_SourceMappingValue::source_name);
			data_source_model_handle.RegisterMember("source_value", &RmlModel_SourceMappingValue::source_value);
		}

		// One time registration for an array of layer data source mappings.
		constructor.RegisterArray<Rml::Vector<RmlModel_SourceMappingValue>>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("client_sources", &m_clientSources);
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
				OnScreenshotClientSourceEvent(m_clientSources[listIndex].source_name);
			}
		});

	// Set initial values for data model
	update();

	return true;
}

void RmlModel_CompositorSources::dispose()
{
	m_compositor= nullptr;
	OnScreenshotClientSourceEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorSources::update()
{
	if (m_compositor == nullptr)
		return;

	// Add clients
	const auto& clientSources = m_compositor->getClientSources();
	m_clientSources.clear();
	for (auto it = clientSources.getMap().begin(); it != clientSources.getMap().end(); it++)
	{
		const GlFrameCompositor::ClientSource* clientSource = it->second;

		m_clientSources.push_back({clientSource->clientId, clientSource->clientInfo.applicationName});
	}
	m_modelHandle.DirtyVariable("client_sources");

	// Add float data source names
	m_floatSources.clear();
	for (auto it = m_compositor->getFloatSources().getMap().begin();
		 it != m_compositor->getFloatSources().getMap().end();
		 it++)
	{
		if (it->first == EMPTY_SOURCE_NAME) continue;
		char formatted_string[64];
		StringUtils::formatString(formatted_string, sizeof(formatted_string), "%.2f", it->second);
		m_floatSources.push_back({it->first, formatted_string});
	}
	m_modelHandle.DirtyVariable("float_sources");

	// Add float2 data source names
	m_float2Sources.clear();
	for (auto it = m_compositor->getFloat2Sources().getMap().begin();
		 it != m_compositor->getFloat2Sources().getMap().end();
		 it++)
	{
		if (it->first == EMPTY_SOURCE_NAME) continue;
		const glm::vec2& vec2 = it->second;
		char formatted_string[64];
		StringUtils::formatString(formatted_string, sizeof(formatted_string), "(%.2f,%.2f)", 
								  vec2.x, vec2.y);
		m_float2Sources.push_back({it->first, formatted_string});
	}
	m_modelHandle.DirtyVariable("float2_sources");

	// Add float3 data source names
	m_float3Sources.clear();
	for (auto it = m_compositor->getFloat3Sources().getMap().begin();
		 it != m_compositor->getFloat3Sources().getMap().end();
		 it++)
	{
		if (it->first == EMPTY_SOURCE_NAME) continue;
		const glm::vec3& vec3 = it->second;
		char formatted_string[64];
		StringUtils::formatString(formatted_string, sizeof(formatted_string), "(%.2f,%.2f,%.2f)", 
								  vec3.x, vec3.y, vec3.z);
		m_float3Sources.push_back({it->first, formatted_string});
	}
	m_modelHandle.DirtyVariable("float3_sources");

	// Add float4 data source names
	m_float4Sources.clear();
	for (auto it = m_compositor->getFloat4Sources().getMap().begin();
		 it != m_compositor->getFloat4Sources().getMap().end();
		 it++)
	{
		if (it->first == EMPTY_SOURCE_NAME) continue;
		const glm::vec4& vec4 = it->second;
		char formatted_string[64];
		StringUtils::formatString(formatted_string, sizeof(formatted_string), "(%.2f,%.2f,%.2f,%.2f)", 
								  vec4.x, vec4.y, vec4.z, vec4.w);
		m_float4Sources.push_back({it->first, formatted_string});
	}
	m_modelHandle.DirtyVariable("float4_sources");

	// Add mat4 data source names
	m_mat4Sources.clear();
	for (auto it = m_compositor->getMat4Sources().getMap().begin();
		 it != m_compositor->getMat4Sources().getMap().end();
		 it++)
	{
		if (it->first == EMPTY_SOURCE_NAME) continue;
		const glm::mat4& mat4 = it->second;
		char formatted_string[256];
		StringUtils::formatString(formatted_string, sizeof(formatted_string), 
								  "|%.2f,%.2f,%.2f,%.2f|\n"
								  "|%.2f,%.2f,%.2f,%.2f|\n"
								  "|%.2f,%.2f,%.2f,%.2f|\n"
								  "|%.2f,%.2f,%.2f,%.2f|",
								  mat4[0][0], mat4[0][1], mat4[0][2], mat4[0][3],
								  mat4[1][0], mat4[1][1], mat4[1][2], mat4[1][3],
								  mat4[2][0], mat4[2][1], mat4[2][2], mat4[2][3],
								  mat4[3][0], mat4[3][1], mat4[3][2], mat4[3][3]);
		m_mat4Sources.push_back({it->first, formatted_string});
	}
	m_modelHandle.DirtyVariable("mat4_sources");

	// Add color texture data source names
	m_colorTextureSources.clear();
	for (auto it = m_compositor->getColorTextureSources().getMap().begin();
		 it != m_compositor->getColorTextureSources().getMap().end();
		 it++)
	{
		if (it->first == EMPTY_SOURCE_NAME) continue;
		if (it->second == nullptr) continue;
		m_colorTextureSources.push_back(it->first);
	}
	m_modelHandle.DirtyVariable("color_texture_sources");
}