#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "NetworkVideoSourceComponent.h"

class GuiPanel_NetworkVideoSourceComponent : public GuiPanel_MikanComponent
{
public:
	virtual bool init(class AppStage* ownerAppStage) override;
	virtual void render(float deltaSeconds) override;

protected:
	NetworkVideoSourceComponentPtr getNetworkVideoSourceComponent() const;
};
