#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "AnchorComponent.h"

class RmlModel_AnchorComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_AnchorComponent();

	virtual bool init(class AppStage* ownerAppStage) override;
};