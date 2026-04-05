#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "StageComponent.h"

class GuiPanel_StageComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_StageComponent() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual void onGui() override;

protected:
	StageComponentPtr getStageComponent() const;

private:
	class AppStage* m_ownerAppStage = nullptr;
};
