#include "RmlModel_FastenerCameraSettings.h"
#include "Constants_FastenerCalibration.h"
#include "ProfileConfig.h"
#include "StringUtils.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceInterface.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_FastenerCameraSettings::init(
	Rml::Context* rmlContext,
	VideoSourceViewConstPtr videoSourceView,
	ProfileConfigConstPtr profileConfig)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "fastener_camera_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("viewpoint_mode", &m_viewpointMode);
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
		"viewpoint_mode_changed",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (ev.GetId() == Rml::EventId::Change)
			{
				const std::string modeName = ev.GetParameter<Rml::String>("value", "");

				// A non-empty "value" means this check box was changed to the checked state
				if (!modeName.empty())
				{
					auto mode = StringUtils::FindEnumValue<eFastenerCalibrationViewpointMode>(
						modeName, k_fastenerCalibrationViewpointModeStrings);

					if (OnViewpointModeChanged)
					{
						OnViewpointModeChanged(mode);
					}
				}
			}
		});

	// Set defaults
	setMenuState(eFastenerCalibrationMenuState::inactive);
	setViewpointMode(eFastenerCalibrationViewpointMode::mixedRealityViewpoint);
	setBrightness(videoSourceView->getVideoProperty(VideoPropertyType::Brightness));
	m_brightnessMin = videoSourceView->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
	m_brightnessMax = videoSourceView->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
	m_brightnessStep = videoSourceView->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);

	return true;
}

void RmlModel_FastenerCameraSettings::dispose()
{
	OnViewpointModeChanged.Clear();
	OnBrightnessChanged.Clear();
	RmlModel::dispose();
}

eFastenerCalibrationMenuState RmlModel_FastenerCameraSettings::getMenuState() const
{
	return StringUtils::FindEnumValue<eFastenerCalibrationMenuState>(
		m_menuState, k_fastenerCalibrationMenuStateStrings);
}

void RmlModel_FastenerCameraSettings::setMenuState(eFastenerCalibrationMenuState newState)
{
	Rml::String newStateString = k_fastenerCalibrationMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

eFastenerCalibrationViewpointMode RmlModel_FastenerCameraSettings::getViewpointMode() const
{
	return StringUtils::FindEnumValue<eFastenerCalibrationViewpointMode>(
		m_viewpointMode, k_fastenerCalibrationViewpointModeStrings);
}

void RmlModel_FastenerCameraSettings::setViewpointMode(eFastenerCalibrationViewpointMode newMode)
{
	Rml::String newModeString = k_fastenerCalibrationViewpointModeStrings[(int)newMode];

	if (m_viewpointMode != newModeString)
	{
		// Update menu state on the data model
		m_viewpointMode = newModeString;
		m_modelHandle.DirtyVariable("viewpoint_mode");
	}
}

int RmlModel_FastenerCameraSettings::getBrightness() const
{
	return m_brightness;
}

void RmlModel_FastenerCameraSettings::setBrightness(int newBrightness)
{
	if (newBrightness != m_brightness)
	{
		m_brightness = newBrightness;
		m_modelHandle.DirtyVariable("brightness");
	}
}