#include "RmlModel_VRTrackingRecenter.h"
#include "Constants_VRTrackingRecenter.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

static const float kMarkerStabilityDuration = 1.0f;

bool RmlModel_VRTrackingRecenter::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "vr_tracking_recenter");
	if (!constructor)
		return false;

	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("calibration_percent", &m_calibrationPercent);
	constructor.Bind("is_current_marker_valid", &m_isCurrentMarkerValid);
	constructor.Bind("is_current_marker_stable", &m_isCurrentMarkerStable);
	constructor.BindEventCallback(
		"begin",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			// Tell the parent app state that we started calibration
			if (OnBeginEvent) OnBeginEvent();
		});
	constructor.BindEventCallback(
		"restart",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			// Tell the parent app state that we restarted
			if (OnRestartEvent) OnRestartEvent();
		});
	constructor.BindEventCallback(
		"cancel",
			[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnCancelEvent) OnCancelEvent();
		});
	constructor.BindEventCallback(
		"return",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnReturnEvent) OnReturnEvent();
		});

	setCalibrationFraction(0.f);
	setMenuState(eVRTrackingRecenterMenuState::inactive);

	return true;
}

void RmlModel_VRTrackingRecenter::dispose()
{
	OnBeginEvent.Clear();
	OnRestartEvent.Clear();
	OnCancelEvent.Clear();
	OnReturnEvent.Clear();
	RmlModel::dispose();
}

eVRTrackingRecenterMenuState RmlModel_VRTrackingRecenter::getMenuState() const
{
	return StringUtils::FindEnumValue<eVRTrackingRecenterMenuState>(
		m_menuState, k_VRTrackingRecenterMenuStateStrings);
}

void RmlModel_VRTrackingRecenter::setMenuState(eVRTrackingRecenterMenuState newState)
{
	Rml::String newStateString= k_VRTrackingRecenterMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

float RmlModel_VRTrackingRecenter::getCalibrationFraction() const
{
	return m_calibrationPercent / 100.f;
}

void RmlModel_VRTrackingRecenter::setCalibrationFraction(const float newFraction)
{
	const float newPercent= newFraction * 100.f;
	if (m_calibrationPercent != newPercent)
	{
		m_calibrationPercent = newPercent;
		m_modelHandle.DirtyVariable("calibration_percent");
	}
}

bool RmlModel_VRTrackingRecenter::getCurrentMarkerValid() const
{
	return m_isCurrentMarkerValid;
}

void RmlModel_VRTrackingRecenter::setCurrentMarkerValid(const bool bNewMarkerValid)
{
	// Reset the stability timer if the image points are no longer valid
	if (m_isCurrentMarkerStable && !bNewMarkerValid)
	{
		setCurrentMarkerStable(false);
	}

	if (m_isCurrentMarkerValid != bNewMarkerValid)
	{
		m_isCurrentMarkerValid = bNewMarkerValid;
		m_modelHandle.DirtyVariable("are_current_image_points_valid");
	}
}

void RmlModel_VRTrackingRecenter::updateMarkerStabilityTimer(float deltaTime)
{
	if (m_isCurrentMarkerValid && !m_isCurrentMarkerStable)
	{
		m_markerStabilityTimer += deltaTime;
		if (m_markerStabilityTimer >= kMarkerStabilityDuration)
		{
			setCurrentMarkerStable(true);
		}
	}
}

void RmlModel_VRTrackingRecenter::setCurrentMarkerStable(const bool bNewImagePointsStability)
{
	if (!bNewImagePointsStability)
	{
		m_markerStabilityTimer = 0.f;
	}

	if (m_isCurrentMarkerStable != bNewImagePointsStability)
	{
		m_isCurrentMarkerStable = bNewImagePointsStability;
		m_modelHandle.DirtyVariable("are_current_image_points_valid");
		if (OnMarkerStabilityChangedEvent)
		{
			OnMarkerStabilityChangedEvent(m_isCurrentMarkerStable);
		}
	}
}

bool RmlModel_VRTrackingRecenter::getCurrentMarkerStable() const
{
	return m_isCurrentMarkerStable;
}