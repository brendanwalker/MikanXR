#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "AnchorComponent.h"

class GuiPanel_AnchorComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_AnchorComponent() = default;
	virtual bool init(class AppStage* ownerAppStage) override;
};
