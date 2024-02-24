#include "RmlModel_DepthMeshCapture.h"
#include "Constants_DepthMeshCapture.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_DepthMeshCapture::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "depth_mesh_capture");
	if (!constructor)
		return false;

	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("are_current_image_points_valid", &m_areCurrentImagePointsValid);
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

	setCurrentImagePointsValid(false);
	setMenuState(eDepthMeshCaptureMenuState::inactive);

	return true;
}

void RmlModel_DepthMeshCapture::dispose()
{
	OnBeginEvent.Clear();
	OnRestartEvent.Clear();
	OnCancelEvent.Clear();
	OnReturnEvent.Clear();
	RmlModel::dispose();
}

bool RmlModel_DepthMeshCapture::getBypassCalibrationFlag() const
{
	return m_bypassCalibrationFlag;
}

void RmlModel_DepthMeshCapture::setBypassCalibrationFlag(const bool bNewFlag)
{
	if (bNewFlag != m_bypassCalibrationFlag)
	{
		m_bypassCalibrationFlag = bNewFlag;

		// Can be called before RmlModel_DepthMeshCapture::init()
		if (m_modelHandle)
		{
			m_modelHandle.DirtyVariable("bypass_calibration_flag");
		}
	}
}

eDepthMeshCaptureMenuState RmlModel_DepthMeshCapture::getMenuState() const
{
	return StringUtils::FindEnumValue<eDepthMeshCaptureMenuState>(
		m_menuState, k_DepthMeshCaptureMenuStateStrings);
}

void RmlModel_DepthMeshCapture::setMenuState(eDepthMeshCaptureMenuState newState)
{
	Rml::String newStateString= k_DepthMeshCaptureMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

bool RmlModel_DepthMeshCapture::getCurrentImagePointsValid() const
{
	return m_areCurrentImagePointsValid;
}

void RmlModel_DepthMeshCapture::setCurrentImagePointsValid(const bool bNewImagePointsValid)
{
	if (m_areCurrentImagePointsValid != bNewImagePointsValid)
	{
		m_areCurrentImagePointsValid = bNewImagePointsValid;
		m_modelHandle.DirtyVariable("are_current_image_points_valid");
	}
}