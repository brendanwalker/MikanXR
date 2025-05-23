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
		"toggle_video",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleVideoEvent) OnToggleVideoEvent();
		});
	constructor.BindEventCallback(
		"toggle_scripting",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
		if (OnToggleScriptingEvent) OnToggleScriptingEvent();
	});
	constructor.BindEventCallback(
		"toggle_sources",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
		if (OnToggleSourcesEvent) OnToggleSourcesEvent();
	});
	constructor.BindEventCallback(
		"toggle_settings",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
		if (OnToggleSettingsEvent) OnToggleSettingsEvent();
	});

	// Set defaults

	return true;
}

void RmlModel_Compositor::dispose()
{
	OnReturnEvent.Clear();
	OnToggleOutlinerEvent.Clear();
	OnToggleLayersEvent.Clear();
	OnToggleVideoEvent.Clear();
	OnToggleScriptingEvent.Clear();
	OnToggleSourcesEvent.Clear();
	OnToggleSettingsEvent.Clear();
	RmlModel::dispose();
}