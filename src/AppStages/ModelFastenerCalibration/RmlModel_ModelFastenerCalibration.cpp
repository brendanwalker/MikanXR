#include "RmlModel_ModelFastenerCalibration.h"
#include "Constants_ModelFastenerCalibration.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_ModelFastenerCalibration::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "model_fastener_calibration");
	if (!constructor)
		return false;

	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("captured_point_count", &m_capturedPointCount);
	constructor.Bind("bypass_calibration_flag", &m_bypassCalibrationFlag);
	constructor.BindEventCallback(
		"ok",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnOkEvent) OnOkEvent();
		});
	constructor.BindEventCallback(
		"redo",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnRedoEvent) OnRedoEvent();
		});
	constructor.BindEventCallback(
		"cancel",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnCancelEvent) OnCancelEvent();
		});

	setCapturedPointCount(0);
	setMenuState(eModelFastenerCalibrationMenuState::inactive);

	return true;
}

void RmlModel_ModelFastenerCalibration::dispose()
{
	OnOkEvent.Clear();
	OnRedoEvent.Clear();
	OnCancelEvent.Clear();
	RmlModel::dispose();
}

eModelFastenerCalibrationMenuState RmlModel_ModelFastenerCalibration::getMenuState() const
{
	return StringUtils::FindEnumValue<eModelFastenerCalibrationMenuState>(
		m_menuState, k_modelFastenerCalibrationMenuStateStrings);
}

void RmlModel_ModelFastenerCalibration::setMenuState(eModelFastenerCalibrationMenuState newState)
{
	Rml::String newStateString= k_modelFastenerCalibrationMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

int RmlModel_ModelFastenerCalibration::getCapturedPointCount() const
{
	return m_capturedPointCount;
}

void RmlModel_ModelFastenerCalibration::setCapturedPointCount(const int newCount)
{
	if (m_capturedPointCount != newCount)
	{
		m_capturedPointCount = newCount;
		m_modelHandle.DirtyVariable("captured_point_count");
	}
}