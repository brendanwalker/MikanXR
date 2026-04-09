#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "CameraComponent.h"

class GuiPanel_CameraComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_CameraComponent(AppStage* ownerAppStage);

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	CameraComponentPtr getCameraComponent() const;

private:
	GuiDataSource_ComboBox m_videoSourceDataSource;
	GuiDataSource_ComboBox m_trackingMountDataSource;
};
