#include "RmlModel_FastenerCalibration.h"
#include "Constants_FastenerCalibration.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_FastenerCalibration::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "fastener_calibration");
	if (!constructor)
		return false;

	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("captured_point_count", &m_capturedPointCount);
	constructor.Bind("bypass_calibration_flag", &m_bypassCalibrationFlag);
	constructor.BindEventCallback(
		"continue",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			// Tell the parent app state that we started calibration
			if (OnContinueEvent) OnContinueEvent();
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

	setCapturedPointCount(0);
	setMenuState(eFastenerCalibrationMenuState::inactive);

	return true;
}

void RmlModel_FastenerCalibration::dispose()
{
	OnContinueEvent.Clear();
	OnRestartEvent.Clear();
	OnCancelEvent.Clear();
	OnReturnEvent.Clear();
	RmlModel::dispose();
}

bool RmlModel_FastenerCalibration::getBypassCalibrationFlag() const
{
	return m_bypassCalibrationFlag;
}

void RmlModel_FastenerCalibration::setBypassCalibrationFlag(const bool bNewFlag)
{
	if (bNewFlag != m_bypassCalibrationFlag)
	{
		m_bypassCalibrationFlag = bNewFlag;

		// Can be called before RmlModel_FastenerCalibration::init()
		if (m_modelHandle)
		{
			m_modelHandle.DirtyVariable("bypass_calibration_flag");
		}
	}
}

eFastenerCalibrationMenuState RmlModel_FastenerCalibration::getMenuState() const
{
	return StringUtils::FindEnumValue<eFastenerCalibrationMenuState>(
		m_menuState, k_fastenerCalibrationMenuStateStrings);
}

void RmlModel_FastenerCalibration::setMenuState(eFastenerCalibrationMenuState newState)
{
	Rml::String newStateString= k_fastenerCalibrationMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

int RmlModel_FastenerCalibration::getCapturedPointCount() const
{
	return m_capturedPointCount;
}

void RmlModel_FastenerCalibration::setCapturedPointCount(const int newCount)
{
	if (m_capturedPointCount != newCount)
	{
		m_capturedPointCount = newCount;
		m_modelHandle.DirtyVariable("captured_point_count");
	}
}