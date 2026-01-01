#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_TextureSourceSettings : public RmlModel
{
public:
	RmlModel_TextureSourceSettings() = default;

	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	SinglecastDelegate<void()> OnReturnEvent;
};
