#include "RmlModel_AlignmentCalibration.h"
#include "Constants_AlignmentCalibration.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

static const float kChessboardStabilityDuration = 1.0f;

bool RmlModel_AlignmentCalibration::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "alignment_calibration");
	if (!constructor)
		return false;

	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("calibration_percent", &m_calibrationPercent);
	constructor.Bind("bypass_calibration_flag", &m_bypassCalibrationFlag);
	constructor.Bind("is_current_chessboard_valid", &m_isCurrentChessboardValid);
	constructor.Bind("is_current_chessboard_stable", &m_isCurrentChessboardStable);
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
	setMenuState(eAlignmentCalibrationMenuState::inactive);

	return true;
}

void RmlModel_AlignmentCalibration::dispose()
{
	OnBeginEvent.Clear();
	OnRestartEvent.Clear();
	OnCancelEvent.Clear();
	OnReturnEvent.Clear();
	RmlModel::dispose();
}

bool RmlModel_AlignmentCalibration::getBypassCalibrationFlag() const
{
	return m_bypassCalibrationFlag;
}

void RmlModel_AlignmentCalibration::setBypassCalibrationFlag(const bool bNewFlag)
{
	if (bNewFlag != m_bypassCalibrationFlag)
	{
		m_bypassCalibrationFlag = bNewFlag;

		// Can be called before RmlModel_AlignmentCalibration::init()
		if (m_modelHandle)
		{
			m_modelHandle.DirtyVariable("bypass_calibration_flag");
		}
	}
}

eAlignmentCalibrationMenuState RmlModel_AlignmentCalibration::getMenuState() const
{
	return StringUtils::FindEnumValue<eAlignmentCalibrationMenuState>(
		m_menuState, k_alignmentCalibrationMenuStateStrings);
}

void RmlModel_AlignmentCalibration::setMenuState(eAlignmentCalibrationMenuState newState)
{
	Rml::String newStateString= k_alignmentCalibrationMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

float RmlModel_AlignmentCalibration::getCalibrationFraction() const
{
	return m_calibrationPercent / 100.f;
}

void RmlModel_AlignmentCalibration::setCalibrationFraction(const float newFraction)
{
	const float newPercent= newFraction * 100.f;
	if (m_calibrationPercent != newPercent)
	{
		m_calibrationPercent = newPercent;
		m_modelHandle.DirtyVariable("calibration_percent");
	}
}

bool RmlModel_AlignmentCalibration::getCurrentChessboardValid() const
{
	return m_isCurrentChessboardValid;
}

void RmlModel_AlignmentCalibration::setCurrentChessboardValid(const bool bNewImagePointsValid)
{
	// Reset the stability timer if the image points are no longer valid
	if (m_isCurrentChessboardStable && !bNewImagePointsValid)
	{
		setCurrentChessboardStable(false);
	}

	if (m_isCurrentChessboardValid != bNewImagePointsValid)
	{
		m_isCurrentChessboardValid = bNewImagePointsValid;
		m_modelHandle.DirtyVariable("is_current_chessboard_valid");
	}
}

void RmlModel_AlignmentCalibration::updateChessboardStabilityTimer(float deltaTime)
{
	if (m_isCurrentChessboardValid && !m_isCurrentChessboardStable)
	{
		m_chessboardStabilityTimer += deltaTime;
		if (m_chessboardStabilityTimer >= kChessboardStabilityDuration)
		{
			setCurrentChessboardStable(true);
		}
	}
}

void RmlModel_AlignmentCalibration::setCurrentChessboardStable(const bool bNewImagePointsStability)
{
	if (!bNewImagePointsStability)
	{
		m_chessboardStabilityTimer = 0.f;
	}

	if (m_isCurrentChessboardStable != bNewImagePointsStability)
	{
		m_isCurrentChessboardStable = bNewImagePointsStability;
		m_modelHandle.DirtyVariable("is_current_chessboard_stable");
		if (OnChessboardStabilityChangedEvent)
		{
			OnChessboardStabilityChangedEvent(m_isCurrentChessboardStable);
		}
	}
}

bool RmlModel_AlignmentCalibration::getCurrentChessboardStable() const
{
	return m_isCurrentChessboardStable;
}