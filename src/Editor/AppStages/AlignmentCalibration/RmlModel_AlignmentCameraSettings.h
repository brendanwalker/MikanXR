#pragma once

#include "ObjectSystemConfigFwd.h"
#include "SinglecastDelegate.h"
#include "Shared/RmlModel.h"
#include "Constants_AlignmentCalibration.h"

class ProjectConfig;

class VideoSourceComponent;
typedef std::shared_ptr<const VideoSourceComponent> VideoSourceComponentConstPtr;

class RmlModel_AlignmentCameraSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		VideoSourceComponentConstPtr videoSourceComponent,
		ProjectConfigConstPtr profileConfig);
	virtual void dispose() override;

	eAlignmentCalibrationMenuState getMenuState() const;
	void setMenuState(eAlignmentCalibrationMenuState newState);

	eAlignmentCalibrationViewpointMode getViewpointMode() const;
	void setViewpointMode(eAlignmentCalibrationViewpointMode newMode);

	int getVRFrameDelay() const;
	void setVRFrameDelay(int newFrameDelay);

	SinglecastDelegate<void(eAlignmentCalibrationViewpointMode)> OnViewpointModeChanged;
	SinglecastDelegate<void(int)> OnVRFrameDelayChanged;

private:
	// Model Variables
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
	int m_vrFrameDelay= 0;
};
