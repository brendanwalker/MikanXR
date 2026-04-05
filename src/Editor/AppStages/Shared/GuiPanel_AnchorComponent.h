#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "AnchorComponent.h"

class GuiPanel_AnchorComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_AnchorComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}
	virtual bool init() override;
};
