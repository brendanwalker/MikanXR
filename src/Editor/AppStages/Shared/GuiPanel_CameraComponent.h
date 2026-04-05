#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "CameraComponent.h"

class GuiPanel_CameraComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_CameraComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	virtual bool init() override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void onGui() override;

protected:
	CameraComponentPtr getCameraComponent() const;

private:
	std::vector<int> m_trackingMountIdList;
	std::vector<int> m_videoSourceIdList;
};
