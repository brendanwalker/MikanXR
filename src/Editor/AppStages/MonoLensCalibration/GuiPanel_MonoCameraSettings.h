#pragma once

#include "VideoDisplayConstants.h"
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_MonoCameraSettings : public GuiPanel
{
public:
	bool init(AppStage* ownerAppStage);
	virtual void onGui() override;

	void setVideoDisplayMode(eVideoDisplayMode mode) { m_videoDisplayMode = (int)mode; }

	std::function<void(eVideoDisplayMode)> OnVideoDisplayModeChanged;

private:
	int m_videoDisplayMode = (int)eVideoDisplayMode::mode_bgr;
};
