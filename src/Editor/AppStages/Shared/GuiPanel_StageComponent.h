#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "StageComponent.h"

class GuiPanel_StageComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_StageComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	virtual bool init() override;
	virtual void onGui() override;

protected:
	StageComponentPtr getStageComponent() const;
};
