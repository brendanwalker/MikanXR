#pragma once

#include "ComponentFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"

class RmlModel_ProjectSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		ProjectConfigPtr profileConfig,
		StencilObjectSystemPtr stencilSystem,
		CompositorDefinitionPtr compositorDefinition);
	virtual void dispose() override;

private:
	bool m_bIsStreaming = false;
	Rml::String m_spoutOutputName;

	bool m_bRenderOrigin= false;
	bool m_bRenderAnchors= false;
	bool m_bRenderStencils= false;

	ProjectConfigPtr m_project;
	StencilObjectSystemWeakPtr m_stencilSystem;
	CompositorDefinitionPtr m_compositorDefinition;
};
