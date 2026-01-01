#include "RmlModel_VideoSourceSettings.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_VideoSourceSettings::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "VideoSourceSettings");
	if (!constructor)
		return false;

	constructor.BindEventCallback(
		"return",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnReturnEvent) OnReturnEvent();
		});

	return true;
}

void RmlModel_VideoSourceSettings::dispose()
{
	OnReturnEvent.Clear();

	RmlModel::dispose();
}