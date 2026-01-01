#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_VideoSourceSettings : public RmlModel
{
public:
	RmlModel_VideoSourceSettings() = default;

	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	SinglecastDelegate<void()> OnReturnEvent;
};
