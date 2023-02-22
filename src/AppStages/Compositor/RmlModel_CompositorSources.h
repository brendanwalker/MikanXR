#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_SourceMappingValue
{
	Rml::String source_name;
	Rml::String source_value;
};

class RmlModel_CompositorSources : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext, 
		const class GlFrameCompositor* compositor);
	virtual void dispose() override;
	virtual void update() override;

	SinglecastDelegate<void(const Rml::String& clientSourceName)> OnScreenshotClientSourceEvent;

private:
	const GlFrameCompositor* m_compositor= nullptr;

	Rml::Vector<RmlModel_SourceMappingValue> m_clientSources;
	Rml::Vector<RmlModel_SourceMappingValue> m_floatSources;
	Rml::Vector<RmlModel_SourceMappingValue> m_float2Sources;
	Rml::Vector<RmlModel_SourceMappingValue> m_float3Sources;
	Rml::Vector<RmlModel_SourceMappingValue> m_float4Sources;
	Rml::Vector<RmlModel_SourceMappingValue> m_mat4Sources;
	Rml::Vector<Rml::String> m_colorTextureSources;

	static bool s_bHasRegisteredTypes;
};
