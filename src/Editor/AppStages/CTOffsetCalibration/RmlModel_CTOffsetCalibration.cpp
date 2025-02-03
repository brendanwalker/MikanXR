#include "RmlModel_CTOffsetCalibration.h"
#include "Constants_CTOffsetCalibration.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CTOffsetCalibration::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "ctOffset_calibration");
	if (!constructor)
		return false;

	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("calibration_percent", &m_calibrationPercent);
	constructor.Bind("bypass_calibration_flag", &m_bypassCalibrationFlag);

	constructor.BindEventCallback(
		"continue",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			// Tell the parent app state that we are advancing to the next state
			if (OnContinueEvent) OnContinueEvent();
		});
	constructor.BindEventCallback(
		"capture",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnCaptureEvent) OnCaptureEvent();
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

	setCalibrationFraction(0.f);
	setMenuState(eCTOffsetCalibrationMenuState::inactive);

	return true;
}

void RmlModel_CTOffsetCalibration::dispose()
{
	OnContinueEvent.Clear();
	OnCaptureEvent.Clear();
	OnRestartEvent.Clear();
	OnCancelEvent.Clear();
	RmlModel::dispose();
}

bool RmlModel_CTOffsetCalibration::getBypassCalibrationFlag() const
{
	return m_bypassCalibrationFlag;
}

void RmlModel_CTOffsetCalibration::setBypassCalibrationFlag(const bool bNewFlag)
{
	if (bNewFlag != m_bypassCalibrationFlag)
	{
		m_bypassCalibrationFlag = bNewFlag;

		// Can be called before RmlModel_CTOffsetCalibration::init()
		if (m_modelHandle)
		{
			m_modelHandle.DirtyVariable("bypass_calibration_flag");
		}
	}
}

eCTOffsetCalibrationMenuState RmlModel_CTOffsetCalibration::getMenuState() const
{
	return StringUtils::FindEnumValue<eCTOffsetCalibrationMenuState>(
		m_menuState, k_CTOffsetCalibrationMenuStateStrings);
}

void RmlModel_CTOffsetCalibration::setMenuState(eCTOffsetCalibrationMenuState newState)
{
	Rml::String newStateString= k_CTOffsetCalibrationMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

float RmlModel_CTOffsetCalibration::getCalibrationFraction() const
{
	return m_calibrationPercent / 100.f;
}

void RmlModel_CTOffsetCalibration::setCalibrationFraction(const float newFraction)
{
	const float newPercent= newFraction * 100.f;
	if (m_calibrationPercent != newPercent)
	{
		m_calibrationPercent = newPercent;
		m_modelHandle.DirtyVariable("calibration_percent");
	}
}