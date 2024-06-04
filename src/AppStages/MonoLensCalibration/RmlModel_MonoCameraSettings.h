#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "VideoDisplayConstants.h"

class RmlModel_MonoCameraSettings : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);

	void setVideoDisplayMode(eVideoDisplayMode newMode, bool bMarkDirty= true);

	SinglecastDelegate<void(eVideoDisplayMode)> OnVideoDisplayModeChanged;

private:
	// Model Variables
	Rml::String m_videoDisplayMode;
};
