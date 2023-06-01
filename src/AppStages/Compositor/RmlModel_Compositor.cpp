#include "RmlModel_Compositor.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_Compositor::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor");
	if (!constructor)
		return false;

	// Register Data Model Fields

	// Bind data model callbacks
	constructor.BindEventCallback(
		"return",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnReturnEvent) OnReturnEvent();
		});
	constructor.BindEventCallback(
		"toggle_outliner",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleOutlinerEvent) OnToggleOutlinerEvent();
		});
	constructor.BindEventCallback(
		"toggle_layers",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleLayersEvent) OnToggleLayersEvent();
		});
	constructor.BindEventCallback(
		"toggle_recording",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleRecordingEvent) OnToggleRecordingEvent();
		});
	constructor.BindEventCallback(
		"toggle_scripting",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
		if (OnToggleScriptingEvent) OnToggleScriptingEvent();
	});
	constructor.BindEventCallback(
		"toggle_anchors",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
		if (OnToggleAnchorsEvent) OnToggleAnchorsEvent();
	});
	constructor.BindEventCallback(
		"toggle_quad_stencils",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
		if (OnToggleQuadStencilsEvent) OnToggleQuadStencilsEvent();
	});
	constructor.BindEventCallback(
		"toggle_box_stencils",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
		if (OnToggleBoxStencilsEvent) OnToggleBoxStencilsEvent();
	});
	constructor.BindEventCallback(
		"toggle_model_stencils",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleModelStencilsEvent) OnToggleModelStencilsEvent();
		});
	constructor.BindEventCallback(
		"toggle_sources",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
		if (OnToggleSourcesEvent) OnToggleSourcesEvent();
	});

	// Set defaults

	return true;
}

void RmlModel_Compositor::dispose()
{
	OnReturnEvent.Clear();
	OnToggleOutlinerEvent.Clear();
	OnToggleLayersEvent.Clear();
	OnToggleRecordingEvent.Clear();
	OnToggleScriptingEvent.Clear();
	OnToggleAnchorsEvent.Clear();
	OnToggleQuadStencilsEvent.Clear();
	OnToggleBoxStencilsEvent.Clear();
	OnToggleModelStencilsEvent.Clear();
	OnToggleSourcesEvent.Clear();
	RmlModel::dispose();
}