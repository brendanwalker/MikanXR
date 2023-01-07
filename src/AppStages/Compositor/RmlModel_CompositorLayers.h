#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorLayer
{
	Rml::String client_id;
	Rml::String app_name;
	Rml::String alpha_mode;
};

class RmlModel_CompositorLayers : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class GlFrameCompositor* compositor);
	virtual void dispose() override;

	SinglecastDelegate<void(int layerIndex, eCompositorLayerAlphaMode)> OnLayerAlphaModeChangedEvent;
	SinglecastDelegate<void(int layerIndex)> OnScreenshotLayerEvent;

	void rebuildLayers(const class GlFrameCompositor* compositor);

private:
	Rml::Vector<Rml::String> m_alphaModes;
	Rml::Vector<RmlModel_CompositorLayer> m_compositorLayers;

	static bool s_bHasRegisteredTypes;
};
