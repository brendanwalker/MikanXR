#include "RmlModel_Project.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_Project::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "project");
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
		"toggle_scenes",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleScenesEvent) OnToggleScenesEvent();
		});
	constructor.BindEventCallback(
		"toggle_stages",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleStagesEvent) OnToggleStagesEvent();
		});
	constructor.BindEventCallback(
		"toggle_sources",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleSourcesEvent) OnToggleSourcesEvent();
		});
	constructor.BindEventCallback(
		"toggle_tracking",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleTrackingEvent) OnToggleTrackingEvent();
		});
	constructor.BindEventCallback(
		"toggle_markers",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleMarkersEvent) OnToggleMarkersEvent();
		});
	constructor.BindEventCallback(
		"toggle_settings",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleSettingsEvent) OnToggleSettingsEvent();
		});

	// Set defaults

	return true;
}

void RmlModel_Project::dispose()
{
	OnReturnEvent.Clear();
	OnToggleScenesEvent.Clear();
	OnToggleStagesEvent.Clear();
	OnToggleSourcesEvent.Clear();
	OnToggleTrackingEvent.Clear();
	OnToggleMarkersEvent.Clear();
	OnToggleSettingsEvent.Clear();

	RmlModel::dispose();
}