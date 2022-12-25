#include "RmlModel_CalibrationCameraSettings.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceInterface.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CalibrationCameraSettings::init(
	Rml::Context* rmlContext,
	VideoSourceViewPtr videoSourceView,
	VideoFrameDistortionView* monoDistortionView)
{
	m_videoSourceView = videoSourceView;
	m_monoDistortionView = monoDistortionView;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "calibration_camera_settings");
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
	m_videoDisplayModes = {"BGR", "Undistorted", "Grayscale"};
	m_videoDisplayMode = (int)eVideoDisplayMode::mode_bgr;
	m_brightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);
	m_brightnessMin = m_videoSourceView->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
	m_brightnessMax = m_videoSourceView->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
	m_brightnessStep = m_videoSourceView->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);

	return true;
}

void RmlModel_CalibrationCameraSettings::update()
{
	if (m_modelHandle.IsVariableDirty("video_display_mode"))
	{
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode(m_videoDisplayMode));
	}
	if (m_modelHandle.IsVariableDirty("brightness"))
	{
		m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, m_brightness, true);
		m_brightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);
	}
}

void RmlModel_CalibrationCameraSettings::setVideoDisplayMode(eVideoDisplayMode newMode)
{
	int intNewMode= int(newMode);

	if (intNewMode != m_videoDisplayMode)
	{
		m_videoDisplayMode= intNewMode;
		m_modelHandle.DirtyVariable("video_display_mode");
		m_monoDistortionView->setVideoDisplayMode(newMode);
	}
}