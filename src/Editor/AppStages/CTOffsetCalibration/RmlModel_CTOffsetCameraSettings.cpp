#include "RmlModel_CTOffsetCameraSettings.h"
#include "Constants_CTOffsetCalibration.h"
#include "ProjectConfig.h"
#include "StringUtils.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceInterface.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CTOffsetCameraSettings::init(
	Rml::Context* rmlContext,
	VideoSourceViewConstPtr videoSourceView,
	ProfileConfigConstPtr profileConfig)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "CTOffset_camera_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("viewpoint_mode", &m_viewpointMode);
	constructor.Bind("vr_frame_delay", &m_vrFrameDelay);	
	constructor.Bind("brightness", &m_brightness);
	constructor.Bind("brightness_min", &m_brightnessMin);
	constructor.Bind("brightness_max", &m_brightnessMax);
	constructor.Bind("brightness_step", &m_brightnessStep);
	constructor.BindEventCallback(
		"brightness_changed",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (ev.GetId() == Rml::EventId::Change)
			{
				const float newBrightness = ev.GetParameter<float>("value", 0.f);

				if (OnBrightnessChanged)
				{
					OnBrightnessChanged(newBrightness);
				}
			}
		});
	constructor.BindEventCallback(
		"vr_frame_delay_changed",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (ev.GetId() == Rml::EventId::Change)
			{
				const float newVRFrameDelay = ev.GetParameter<float>("value", 0.f);

				if (OnVRFrameDelayChanged)
				{
					OnVRFrameDelayChanged(newVRFrameDelay);
				}
			}
		});
	constructor.BindEventCallback(
		"viewpoint_mode_changed",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (ev.GetId() == Rml::EventId::Change)
			{
				const std::string modeName = ev.GetParameter<Rml::String>("value", "");

				// A non-empty "value" means this check box was changed to the checked state
				if (!modeName.empty())
				{
					auto mode = StringUtils::FindEnumValue<eCTOffsetCalibrationViewpointMode>(
						modeName, k_CTOffsetCalibrationViewpointModeStrings);

					if (OnViewpointModeChanged)
					{
						OnViewpointModeChanged(mode);
					}
				}
			}
		});

	// Set defaults
	setMenuState(eCTOffsetCalibrationMenuState::inactive);
	setViewpointMode(eCTOffsetCalibrationViewpointMode::cameraViewpoint);
	setVRFrameDelay(profileConfig->getVRFrameDelay());
	setBrightness(videoSourceView->getVideoProperty(VideoPropertyType::Brightness));
	m_brightnessMin = videoSourceView->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
	m_brightnessMax = videoSourceView->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
	m_brightnessStep = videoSourceView->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);

	return true;
}

void RmlModel_CTOffsetCameraSettings::dispose()
{

	OnViewpointModeChanged.Clear();
	OnBrightnessChanged.Clear();
	OnVRFrameDelayChanged.Clear();
	RmlModel::dispose();
}

eCTOffsetCalibrationMenuState RmlModel_CTOffsetCameraSettings::getMenuState() const
{
	return StringUtils::FindEnumValue<eCTOffsetCalibrationMenuState>(
		m_menuState, k_CTOffsetCalibrationMenuStateStrings);
}

void RmlModel_CTOffsetCameraSettings::setMenuState(eCTOffsetCalibrationMenuState newState)
{
	Rml::String newStateString = k_CTOffsetCalibrationMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

eCTOffsetCalibrationViewpointMode RmlModel_CTOffsetCameraSettings::getViewpointMode() const
{
	return StringUtils::FindEnumValue<eCTOffsetCalibrationViewpointMode>(
		m_viewpointMode, k_CTOffsetCalibrationViewpointModeStrings);
}

void RmlModel_CTOffsetCameraSettings::setViewpointMode(eCTOffsetCalibrationViewpointMode newMode)
{
	Rml::String newModeString = k_CTOffsetCalibrationViewpointModeStrings[(int)newMode];

	if (m_viewpointMode != newModeString)
	{
		// Update menu state on the data model
		m_viewpointMode = newModeString;
		m_modelHandle.DirtyVariable("viewpoint_mode");
	}
}

int RmlModel_CTOffsetCameraSettings::getBrightness() const
{
	return m_brightness;
}

void RmlModel_CTOffsetCameraSettings::setBrightness(int newBrightness)
{
	if (newBrightness != m_brightness)
	{
		m_brightness = newBrightness;
		m_modelHandle.DirtyVariable("brightness");
	}
}

int RmlModel_CTOffsetCameraSettings::getVRFrameDelay() const
{
	return m_vrFrameDelay;
}

void RmlModel_CTOffsetCameraSettings::setVRFrameDelay(int newFrameDelay)
{
	if (newFrameDelay != m_vrFrameDelay)
	{
		m_vrFrameDelay = newFrameDelay;
		m_modelHandle.DirtyVariable("vr_frame_delay");
	}
}