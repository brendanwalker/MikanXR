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
	SinglecastDelegate<void()> OnToggleVideoEvent;
	SinglecastDelegate<void()> OnToggleScriptingEvent;
	SinglecastDelegate<void()> OnToggleSourcesEvent;
	SinglecastDelegate<void()> OnToggleSettingsEvent;

private:
};
