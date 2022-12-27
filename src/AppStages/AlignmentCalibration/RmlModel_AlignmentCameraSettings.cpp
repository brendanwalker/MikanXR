#include "RmlModel_AlignmentCameraSettings.h"
#include "ProfileConfig.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceInterface.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_AlignmentCameraSettings::init(
	Rml::Context* rmlContext,
	VideoSourceViewConstPtr videoSourceView,
	const ProfileConfig* profileConfig)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "alignment_camera_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("video_display_modes", &m_videoDisplayModes);
	constructor.Bind("video_display_mode", &m_videoDisplayMode);
	constructor.Bind("show_video_display_mode", &m_bShowVideoDisplayMode);
	constructor.Bind("viewpoint_modes", &m_viewpointModes);
	constructor.Bind("viewpoint_mode", &m_viewpointMode);
	constructor.Bind("vr_frame_delay", &m_vrFrameDelay);	
	constructor.Bind("brightness", &m_brightness);
	constructor.Bind("brightness_min", &m_brightnessMin);
	constructor.Bind("brightness_max", &m_brightnessMax);
	constructor.Bind("brightness_step", &m_brightnessStep);

	// Set defaults
	m_videoDisplayModes = {"BGR", "Undistorted", "Grayscale"};
	m_videoDisplayMode = (int)eVideoDisplayMode::mode_bgr;
	m_videoDisplayModes = {"Camera Viewpoint", "VR Viewpoint", "Mixed Reality Viewpoint"};
	m_vrFrameDelay = profileConfig->vrFrameDelay;
	m_viewpointMode = (int)eAlignmentCalibrationViewpointMode::cameraViewpoint;
	m_brightness = videoSourceView->getVideoProperty(VideoPropertyType::Brightness);
	m_brightnessMin = videoSourceView->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
	m_brightnessMax = videoSourceView->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
	m_brightnessStep = videoSourceView->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);

	return true;
}

void RmlModel_AlignmentCameraSettings::update()
{
	if (m_modelHandle.IsVariableDirty("video_display_mode"))
	{
		if (OnVideoDisplayModeChanged) OnVideoDisplayModeChanged(eVideoDisplayMode(m_videoDisplayMode));
	}
	if (m_modelHandle.IsVariableDirty("brightness"))
	{
		if (OnBrightnessChanged) OnBrightnessChanged(m_brightness);
	}
	if (m_modelHandle.IsVariableDirty("vr_frame_delay"))
	{
		if (OnVRFrameDelayChanged) OnVRFrameDelayChanged(m_vrFrameDelay);
	}
}

bool RmlModel_AlignmentCameraSettings::getShowVideoDisplayMode() const
{
	return m_bShowVideoDisplayMode;
}

void RmlModel_AlignmentCameraSettings::setShowVideoDisplayMode(bool bNewFlag)
{
	if (bNewFlag != m_bShowVideoDisplayMode)
	{
		m_bShowVideoDisplayMode = bNewFlag;
		m_modelHandle.DirtyVariable("show_video_display_mode");
	}
}

eVideoDisplayMode RmlModel_AlignmentCameraSettings::getVideoDisplayMode() const
{
	return eVideoDisplayMode(m_videoDisplayMode);
}

void RmlModel_AlignmentCameraSettings::setVideoDisplayMode(eVideoDisplayMode newMode)
{
	int intNewMode = int(newMode);

	if (intNewMode != m_videoDisplayMode)
	{
		m_videoDisplayMode = intNewMode;
		m_modelHandle.DirtyVariable("video_display_mode");
	}
}

eAlignmentCalibrationViewpointMode RmlModel_AlignmentCameraSettings::getViewpointMode() const
{
	return eAlignmentCalibrationViewpointMode(m_viewpointMode);
}

void RmlModel_AlignmentCameraSettings::setViewpointMode(eAlignmentCalibrationViewpointMode newMode)
{
	int intNewMode = int(newMode);

	if (intNewMode != m_viewpointMode)
	{
		m_viewpointMode = intNewMode;
		m_modelHandle.DirtyVariable("viewpoint_mode");
	}
}

int RmlModel_AlignmentCameraSettings::getVRFrameDelay() const
{
	return m_vrFrameDelay;
}

void RmlModel_AlignmentCameraSettings::setVRFrameDelay(int newFrameDelay)
{
	if (newFrameDelay != m_vrFrameDelay)
	{
		m_vrFrameDelay = newFrameDelay;
		m_modelHandle.DirtyVariable("vr_frame_delay");
	}
}