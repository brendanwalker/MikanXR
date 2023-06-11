#pragma once

#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"

class RmlModel_CompositorSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		ProfileConfigPtr profileConfig);
	virtual void dispose() override;

private:
	bool m_bRenderOrigin= false;
	bool m_bRenderAnchors= false;
	bool m_bRenderStencils= false;
	int m_vrFrameDelay= 0;

	ProfileConfigPtr m_profile;
};
