#pragma once

#include "LightSystemFwd.h"
#include "Shared/GuiPanel_MikanComponent.h"

class GuiPanel_RGBSpotLightComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_RGBSpotLightComponent(class AppStage* ownerAppStage)
		: GuiPanel_MikanComponent(ownerAppStage)
	{
	}

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	RGBSpotLightComponentPtr getRGBSpotLightComponent() const;
};
