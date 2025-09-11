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
		StencilObjectSystemPtr stencilSystem);
	virtual void dispose() override;

private:
	bool m_bRenderOrigin= false;
	bool m_bRenderAnchors= false;
	bool m_bRenderStencils= false;

	ProjectConfigWeakPtr m_project;
	StencilObjectSystemWeakPtr m_stencilSystem;
};
