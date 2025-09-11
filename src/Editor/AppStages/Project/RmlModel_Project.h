#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_Project : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	SinglecastDelegate<void()> OnReturnEvent;
	SinglecastDelegate<void()> OnToggleScenesEvent;
	SinglecastDelegate<void()> OnToggleStagesEvent;
	SinglecastDelegate<void()> OnToggleSourcesEvent;
	SinglecastDelegate<void()> OnToggleTrackingEvent;
	SinglecastDelegate<void()> OnToggleMarkersEvent;
	SinglecastDelegate<void()> OnToggleSettingsEvent;

private:
};
