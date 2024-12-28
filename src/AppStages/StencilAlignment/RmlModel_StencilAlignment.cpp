#include "RmlModel_StencilAlignment.h"
#include "Constants_StencilAlignment.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_StencilAlignment::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "stencil_alignment");
	if (!constructor)
		return false;

	constructor.Bind("menu_state", &m_menuState);
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

	setMenuState(eStencilAlignmentMenuState::inactive);

	return true;
}

void RmlModel_StencilAlignment::dispose()
{
	OnOkEvent.Clear();
	OnRedoEvent.Clear();
	OnCancelEvent.Clear();
	RmlModel::dispose();
}

eStencilAlignmentMenuState RmlModel_StencilAlignment::getMenuState() const
{
	return StringUtils::FindEnumValue<eStencilAlignmentMenuState>(
		m_menuState, k_StencilAlignmentMenuStateStrings);
}

void RmlModel_StencilAlignment::setMenuState(eStencilAlignmentMenuState newState)
{
	Rml::String newStateString= k_StencilAlignmentMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}