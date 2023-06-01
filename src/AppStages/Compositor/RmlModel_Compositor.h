#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_Compositor : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	SinglecastDelegate<void()> OnReturnEvent;
	SinglecastDelegate<void()> OnToggleOutlinerEvent;
	SinglecastDelegate<void()> OnToggleLayersEvent;
	SinglecastDelegate<void()> OnToggleRecordingEvent;
	SinglecastDelegate<void()> OnToggleScriptingEvent;
	SinglecastDelegate<void()> OnToggleAnchorsEvent;
	SinglecastDelegate<void()> OnToggleQuadStencilsEvent;
	SinglecastDelegate<void()> OnToggleBoxStencilsEvent;
	SinglecastDelegate<void()> OnToggleModelStencilsEvent;
	SinglecastDelegate<void()> OnToggleSourcesEvent;

private:
};
