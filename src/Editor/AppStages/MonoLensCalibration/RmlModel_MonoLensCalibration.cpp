#include "RmlModel_MonoLensCalibration.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

#include "MonoLensDistortionCalibrator.h"

#include <assert.h>

static const float kImagePointStabilityDuration = 1.0f;

bool RmlModel_MonoLensCalibration::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "mono_lens_calibration");
	if (!constructor)
		return false;

	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("are_current_image_points_valid", &m_areCurrentImagePointsValid);
	constructor.Bind("are_current_image_points_stable", &m_areCurrentImagePointsStable);
	constructor.Bind("calibration_percent", &m_calibrationPercent);
	constructor.Bind("reprojection_error", &m_reprojectionError);
	constructor.Bind("bypass_calibration_flag", &m_bypassCalibrationFlag);
	constructor.BindEventCallback(
		"restart",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnRestartEvent) OnRestartEvent();
		});
	constructor.BindEventCallback(
		"return",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnReturnEvent) OnReturnEvent();
		});
	constructor.BindEventCallback(
		"cancel",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnCancelEvent) OnCancelEvent();
		});

	resetCalibrationState();

	return true;
}

void RmlModel_MonoLensCalibration::dispose()
{
	OnRestartEvent.Clear();
	OnReturnEvent.Clear();
	OnCancelEvent.Clear();
	OnImagePointStabilityChangedEvent.Clear();
	RmlModel::dispose();
}

bool RmlModel_MonoLensCalibration::getBypassCalibrationFlag() const
{
	return m_bypassCalibrationFlag;
}

void RmlModel_MonoLensCalibration::setBypassCalibrationFlag(const bool bNewFlag)
{
	if (bNewFlag != m_bypassCalibrationFlag)
	{
		m_bypassCalibrationFlag = bNewFlag;

		// Can be called before RmlModel_MonoLensCalibration::init()
		if (m_modelHandle)
		{
			m_modelHandle.DirtyVariable("bypass_calibration_flag");
		}
	}
}

eMonoLensCalibrationMenuState RmlModel_MonoLensCalibration::getMenuState() const
{
	return eMonoLensCalibrationMenuState(m_menuState);
}

void RmlModel_MonoLensCalibration::setMenuState(eMonoLensCalibrationMenuState newState)
{
	if (m_menuState != (int)newState)
	{
		// Update menu state on the data model
		m_menuState = (int)newState;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

float RmlModel_MonoLensCalibration::getCalibrationFraction() const
{
	return m_calibrationPercent / 100.f;
}

void RmlModel_MonoLensCalibration::setCalibrationFraction(const float newFraction)
{
	const float newPercent= newFraction * 100.f;
	if (m_calibrationPercent != newPercent)
	{
		m_calibrationPercent = newPercent;
		m_modelHandle.DirtyVariable("calibration_percent");
	}
}

bool RmlModel_MonoLensCalibration::getCurrentImagePointsValid() const
{
	return m_areCurrentImagePointsValid;
}

void RmlModel_MonoLensCalibration::setCurrentImagePointsValid(const bool bNewImagePointsValid)
{
	// Reset the stability timer if the image points are no longer valid
	if (m_areCurrentImagePointsStable && !bNewImagePointsValid)
	{
		setCurrentImagePointsStable(false);
	}

	if (m_areCurrentImagePointsValid != bNewImagePointsValid)
	{
		m_areCurrentImagePointsValid = bNewImagePointsValid;
		m_modelHandle.DirtyVariable("are_current_image_points_valid");
	}
}

void RmlModel_MonoLensCalibration::updateImagePointStabilityTimer(float deltaTime)
{
	if (m_areCurrentImagePointsValid && !m_areCurrentImagePointsStable)
	{
		m_imagePointsStabilityTimer += deltaTime;
		if (m_imagePointsStabilityTimer >= kImagePointStabilityDuration)
		{
			setCurrentImagePointsStable(true);
		}
	}
}

void RmlModel_MonoLensCalibration::setCurrentImagePointsStable(const bool bNewImagePointsStability)
{
	if (!bNewImagePointsStability)
	{
		m_imagePointsStabilityTimer= 0.f;
	}

	if (m_areCurrentImagePointsStable != bNewImagePointsStability)
	{
		m_areCurrentImagePointsStable = bNewImagePointsStability;
		m_modelHandle.DirtyVariable("are_current_image_points_valid");
		if (OnImagePointStabilityChangedEvent)
		{
			OnImagePointStabilityChangedEvent(m_areCurrentImagePointsStable);
		}
	}
}

bool RmlModel_MonoLensCalibration::getCurrentImagePointsStable() const
{
	return m_areCurrentImagePointsStable;
}

float RmlModel_MonoLensCalibration::getReprojectionError() const
{
	return m_reprojectionError;
}

void RmlModel_MonoLensCalibration::setReprojectionError(const float newReprojectionError)
{
	if (newReprojectionError != m_reprojectionError)
	{
		m_reprojectionError = newReprojectionError;
		m_modelHandle.DirtyVariable("reprojection_error");
	}
}

void RmlModel_MonoLensCalibration::resetCalibrationState()
{
	setCurrentImagePointsValid(false);
	setCurrentImagePointsStable(false);
	setCalibrationFraction(0.f);
	setReprojectionError(0.f);
}