#pragma once

#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"

class RmlModel_CompositorSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		ProjectConfigPtr profileConfig,
		CompositorDefinitionPtr compositorDefinition);
	virtual void dispose() override;

private:
	bool m_bIsStreaming = false;
	Rml::String m_spoutOutputName;

	bool m_bRenderOrigin= false;
	bool m_bRenderAnchors= false;
	bool m_bRenderStencils= false;

	ProjectConfigPtr m_project;
	CompositorDefinitionPtr m_compositorDefinition;
};
