#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "NetworkVideoSourceComponent.h"

class GuiPanel_NetworkVideoSourceComponent : public GuiPanel_MikanComponent
{
public:
	virtual bool init(class AppStage* ownerAppStage) override;
	virtual void onGui() override;

protected:
	NetworkVideoSourceComponentPtr getNetworkVideoSourceComponent() const;
};
