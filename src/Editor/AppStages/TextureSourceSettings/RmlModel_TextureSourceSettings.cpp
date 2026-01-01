#include "RmlModel_TextureSourceSettings.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_TextureSourceSettings::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "TextureSourceSettings");
	if (!constructor)
		return false;

	constructor.BindEventCallback(
		"return",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnReturnEvent) OnReturnEvent();
		});

	return true;
}

void RmlModel_TextureSourceSettings::dispose()
{
	OnReturnEvent.Clear();

	RmlModel::dispose();
}