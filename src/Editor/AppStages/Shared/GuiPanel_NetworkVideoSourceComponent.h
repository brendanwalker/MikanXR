#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "NetworkVideoSourceComponent.h"

class GuiPanel_NetworkVideoSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_NetworkVideoSourceComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}
	virtual bool init() override;
	virtual void onGui() override;

protected:
	NetworkVideoSourceComponentPtr getNetworkVideoSourceComponent() const;
};
