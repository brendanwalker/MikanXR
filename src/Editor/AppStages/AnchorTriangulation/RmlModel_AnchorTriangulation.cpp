#include "RmlModel_AnchorTriangulation.h"
#include "Constants_AnchorTriangulation.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_AnchorTriangulation::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "anchor_triangulation");
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
	setMenuState(eAnchorTriangulationMenuState::inactive);

	return true;
}

void RmlModel_AnchorTriangulation::dispose()
{
	OnOkEvent.Clear();
	OnRedoEvent.Clear();
	OnCancelEvent.Clear();
	RmlModel::dispose();
}

bool RmlModel_AnchorTriangulation::getBypassCalibrationFlag() const
{
	return m_bypassCalibrationFlag;
}

void RmlModel_AnchorTriangulation::setBypassCalibrationFlag(const bool bNewFlag)
{
	if (bNewFlag != m_bypassCalibrationFlag)
	{
		m_bypassCalibrationFlag = bNewFlag;

		// Can be called before RmlModel_AnchorTriangulation::init()
		if (m_modelHandle)
		{
			m_modelHandle.DirtyVariable("bypass_calibration_flag");
		}
	}
}

eAnchorTriangulationMenuState RmlModel_AnchorTriangulation::getMenuState() const
{
	return StringUtils::FindEnumValue<eAnchorTriangulationMenuState>(
		m_menuState, k_AnchorTriangulationMenuStateStrings);
}

void RmlModel_AnchorTriangulation::setMenuState(eAnchorTriangulationMenuState newState)
{
	Rml::String newStateString= k_AnchorTriangulationMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

int RmlModel_AnchorTriangulation::getCapturedPointCount() const
{
	return m_capturedPointCount;
}

void RmlModel_AnchorTriangulation::setCapturedPointCount(const int newCount)
{
	if (m_capturedPointCount != newCount)
	{
		m_capturedPointCount = newCount;
		m_modelHandle.DirtyVariable("captured_point_count");
	}
}