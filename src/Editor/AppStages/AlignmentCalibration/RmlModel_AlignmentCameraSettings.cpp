#include "CameraComponent.h"
#include "RmlModel_AlignmentCameraSettings.h"
#include "Constants_AlignmentCalibration.h"
#include "StringUtils.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceInterface.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_AlignmentCameraSettings::init(
	Rml::Context* rmlContext,
	CameraDefinitionConstPtr cameraDefinition)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "alignment_camera_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("viewpoint_mode", &m_viewpointMode);
	constructor.Bind("vr_frame_delay", &m_vrFrameDelay);	
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
					auto mode = StringUtils::FindEnumValue<eAlignmentCalibrationViewpointMode>(
						modeName, k_alignmentCalibrationViewpointModeStrings);

					if (OnViewpointModeChanged)
					{
						OnViewpointModeChanged(mode);
					}
				}
			}
		});

	// Set defaults
	setMenuState(eAlignmentCalibrationMenuState::inactive);
	setViewpointMode(eAlignmentCalibrationViewpointMode::cameraViewpoint);
	setVRFrameDelay(cameraDefinition->getTrackingFrameDelay());

	return true;
}

void RmlModel_AlignmentCameraSettings::dispose()
{
	OnViewpointModeChanged.Clear();
	OnVRFrameDelayChanged.Clear();
	RmlModel::dispose();
}

eAlignmentCalibrationMenuState RmlModel_AlignmentCameraSettings::getMenuState() const
{
	return StringUtils::FindEnumValue<eAlignmentCalibrationMenuState>(
		m_menuState, k_alignmentCalibrationMenuStateStrings);
}

void RmlModel_AlignmentCameraSettings::setMenuState(eAlignmentCalibrationMenuState newState)
{
	Rml::String newStateString = k_alignmentCalibrationMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

eAlignmentCalibrationViewpointMode RmlModel_AlignmentCameraSettings::getViewpointMode() const
{
	return StringUtils::FindEnumValue<eAlignmentCalibrationViewpointMode>(
		m_viewpointMode, k_alignmentCalibrationViewpointModeStrings);
}

void RmlModel_AlignmentCameraSettings::setViewpointMode(eAlignmentCalibrationViewpointMode newMode)
{
	Rml::String newModeString = k_alignmentCalibrationViewpointModeStrings[(int)newMode];

	if (m_viewpointMode != newModeString)
	{
		// Update menu state on the data model
		m_viewpointMode = newModeString;
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