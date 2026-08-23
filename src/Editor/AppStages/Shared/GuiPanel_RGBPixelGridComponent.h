#pragma once

#include "LightSystemFwd.h"
#include "Shared/GuiPanel_MikanComponent.h"

class GuiPanel_RGBPixelGridComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_RGBPixelGridComponent(class AppStage* ownerAppStage)
		: GuiPanel_MikanComponent(ownerAppStage)
	{
	}

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	RGBPixelGridComponentPtr getRGBPixelGridComponent() const;
};
