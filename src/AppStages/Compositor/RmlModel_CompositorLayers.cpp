#include "RmlModel_CompositorLayers.h"
#include "GlFrameCompositor.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Event.h>

bool RmlModel_CompositorLayers::s_bHasRegisteredTypes= false;

bool RmlModel_CompositorLayers::init(
	Rml::Context* rmlContext,
	const GlFrameCompositor* compositor)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_layers");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorLayer>())
		{
			layer_model_handle.RegisterMember("client_id", &RmlModel_CompositorLayer::client_id);
			layer_model_handle.RegisterMember("app_name", &RmlModel_CompositorLayer::app_name);
			layer_model_handle.RegisterMember("alpha_mode", &RmlModel_CompositorLayer::alpha_mode);
		}

		// One time registration for an array of compositor layer.
		constructor.RegisterArray<decltype(m_compositorLayers)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("alpha_modes", &m_alphaModes);
	constructor.Bind("layers", &m_compositorLayers);	

	// Bind data model callbacks
	constructor.BindEventCallback(
		"screenshot",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int listIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnScreenshotLayerEvent && listIndex >= 0) OnScreenshotLayerEvent(listIndex);
		});
	constructor.BindEventCallback(
		"set_layer_alpha_mode",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int listIndex = (arguments.size() >= 1 ? arguments[0].Get<int>(-1) : -1);
			const std::string alphaModeString = ev.GetParameter<Rml::String>("value", "");
			const eCompositorLayerAlphaMode alphaMode= 
				StringUtils::FindEnumValue<eCompositorLayerAlphaMode>(
					alphaModeString, k_compositorLayerAlphaStrings);

			if (OnLayerAlphaModeChangedEvent && listIndex >= 0 && alphaMode != eCompositorLayerAlphaMode::INVALID) 
			{
				OnLayerAlphaModeChangedEvent(listIndex, alphaMode);
			}
		});

	// Set initial values for data model
	for (int modeIndex = 0; modeIndex < (int)eCompositorLayerAlphaMode::COUNT; ++modeIndex)
	{
		m_alphaModes.push_back(k_compositorLayerAlphaStrings[modeIndex]);
	}
	rebuildLayers(compositor);

	return true;
}

void RmlModel_CompositorLayers::dispose()
{
	OnLayerAlphaModeChangedEvent.Clear();
	OnScreenshotLayerEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorLayers::rebuildLayers(
	const GlFrameCompositor* compositor)
{
	m_compositorLayers.clear();
	for (const auto& layer : compositor->getLayers())
	{
		const RmlModel_CompositorLayer uiLayer=
		{
			layer.clientId,
			layer.clientInfo.applicationName,
			k_compositorLayerAlphaStrings[(int)layer.alphaMode]
		};
		m_compositorLayers.push_back(uiLayer);
	}
}