#include "RmlModel_MonoLensCalibration.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

#include "MonoLensDistortionCalibrator.h"

#include <assert.h>

bool RmlModel_MonoLensCalibration::init(
	Rml::Context* rmlContext,
	MonoLensDistortionCalibrator* monoLensCalibrator)
{
	m_monoLensCalibrator= monoLensCalibrator;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "mono_lens_calibration");
	if (!constructor)
		return false;

	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("are_current_image_points_valid", &m_areCurrentImagePointsValid);
	constructor.Bind("calibration_progress", &m_calibrationProgress);
	constructor.Bind("reprojection_error", &m_reprojectionError);
	constructor.Bind("bypass_calibration_flag", &m_bypassCalibrationFlag);
	constructor.BindEventCallback(
		"restart",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			// Clear out data model calibration state
			resetCalibrationState();

			if (OnRestartEvent) OnRestartEvent();
		});
	constructor.BindEventCallback(
		"goto_main_menu",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnGotoMainMenuEvent) OnGotoMainMenuEvent();
		});

	resetCalibrationState();

	return true;
}

void RmlModel_MonoLensCalibration::dispose()
{
	OnRestartEvent.Clear();
	OnGotoMainMenuEvent.Clear();
	RmlModel::dispose();
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

void RmlModel_MonoLensCalibration::setCalibrationProgress(const float newProgress)
{
	if (m_calibrationProgress != newProgress)
	{
		m_calibrationProgress = newProgress;
		m_modelHandle.DirtyVariable("calibration_progress");
	}
}

void RmlModel_MonoLensCalibration::setCurrentImagePointsValid(const bool bNewImagePointsValid)
{
	if (m_areCurrentImagePointsValid != bNewImagePointsValid)
	{
		m_areCurrentImagePointsValid = bNewImagePointsValid;
		m_modelHandle.DirtyVariable("are_current_image_points_valid");
	}
}

void RmlModel_MonoLensCalibration::setReprojectionError(const float newReprojectionError)
{
	if (newReprojectionError != m_reprojectionError)
	{
		m_reprojectionError = newReprojectionError;
		m_modelHandle.DirtyVariable("reprojection_error");
	}
}

bool RmlModel_MonoLensCalibration::getBypassCalibrationFlag() const
{
	return m_bypassCalibrationFlag;
}

void RmlModel_MonoLensCalibration::setBypassCalibrationFlag(const bool bNewFlag)
{
	if (bNewFlag != m_bypassCalibrationFlag)
	{
		m_bypassCalibrationFlag= bNewFlag;

		// Can be called before RmlModel_MonoLensCalibration::init()
		if (m_modelHandle)
		{
			m_modelHandle.DirtyVariable("bypass_calibration_flag");
		}
	}
}

void RmlModel_MonoLensCalibration::resetCalibrationState()
{
	setCurrentImagePointsValid(false);
	setCalibrationProgress(0.f);
	setReprojectionError(0.f);
}

void RmlModel_MonoLensCalibration::update()
{
	switch ((eMonoLensCalibrationMenuState)m_menuState)
	{
		case eMonoLensCalibrationMenuState::inactive:
			{

			} break;

		case eMonoLensCalibrationMenuState::capture:
			{
				assert(m_monoLensCalibrator != nullptr);

				// Update calibration progress data binding
				setCalibrationProgress(m_monoLensCalibrator->computeCalibrationProgress() * 100.f);

				// Update image points valid flag data binding
				setCurrentImagePointsValid(m_monoLensCalibrator->areCurrentImagePointsValid());
			} break;

		case eMonoLensCalibrationMenuState::processingCalibration:
			{

			} break;

		case eMonoLensCalibrationMenuState::testCalibration:
			{
				// Update re-projection error
				setReprojectionError(m_monoLensCalibrator->getReprojectionError());

			} break;

		case eMonoLensCalibrationMenuState::failedCalibration:
			{

			} break;

		case eMonoLensCalibrationMenuState::failedVideoStartStreamRequest:
			{

			} break;

		default:
			assert(0 && "unreachable");
	}
}