#include "RmlModel_AlignmentCalibration.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

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
	return eAlignmentCalibrationMenuState(m_menuState);
}

void RmlModel_AlignmentCalibration::setMenuState(eAlignmentCalibrationMenuState newState)
{
	if (m_menuState != (int)newState)
	{
		// Update menu state on the data model
		m_menuState = (int)newState;
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