#include "RmlModel_MonoCameraSettings.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceInterface.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_MonoCameraSettings::init(
	Rml::Context* rmlContext,
	VideoSourceViewConstPtr videoSourceView)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "mono_camera_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("video_display_modes", &m_videoDisplayModes);
	constructor.Bind("video_display_mode", &m_videoDisplayMode);
	constructor.Bind("brightness", &m_brightness);
	constructor.Bind("brightness_min", &m_brightnessMin);
	constructor.Bind("brightness_max", &m_brightnessMax);
	constructor.Bind("brightness_step", &m_brightnessStep);

	// Set defaults
	m_videoDisplayModes = {"BGR", "Undistorted", "Grayscale", "Depth"};
	m_videoDisplayMode = (int)eVideoDisplayMode::mode_bgr;
	m_brightness = videoSourceView->getVideoProperty(VideoPropertyType::Brightness);
	m_brightnessMin = videoSourceView->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
	m_brightnessMax = videoSourceView->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
	m_brightnessStep = videoSourceView->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);

	return true;
}

void RmlModel_MonoCameraSettings::update()
{
	if (m_modelHandle.IsVariableDirty("video_display_mode"))
	{
		if (OnVideoDisplayModeChanged) OnVideoDisplayModeChanged(eVideoDisplayMode(m_videoDisplayMode));
	}
	if (m_modelHandle.IsVariableDirty("brightness"))
	{
		if (OnBrightnessChanged) OnBrightnessChanged(m_brightness);
	}
}

eVideoDisplayMode RmlModel_MonoCameraSettings::getVideoDisplayMode() const
{
	return eVideoDisplayMode(m_videoDisplayMode);
}

void RmlModel_MonoCameraSettings::setVideoDisplayMode(eVideoDisplayMode newMode)
{
	int intNewMode= int(newMode);

	if (intNewMode != m_videoDisplayMode)
	{
		m_videoDisplayMode= intNewMode;
		m_modelHandle.DirtyVariable("video_display_mode");
	}
}