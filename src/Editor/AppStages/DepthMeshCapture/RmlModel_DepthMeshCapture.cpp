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
	constructor.BindEventCallback(
		"continue",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnContinueEvent) OnContinueEvent();
		});
	constructor.BindEventCallback(
		"restart",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnRestartEvent) OnRestartEvent();
		});
	constructor.BindEventCallback(
		"cancel",
			[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnCancelEvent) OnCancelEvent();
		});

	setMenuState(eDepthMeshCaptureMenuState::inactive);

	return true;
}

void RmlModel_DepthMeshCapture::dispose()
{
	OnContinueEvent.Clear();
	OnRestartEvent.Clear();
	OnCancelEvent.Clear();
	RmlModel::dispose();
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