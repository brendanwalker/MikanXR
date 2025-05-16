#include "RmlModel_CompositorSettings.h"
#include "AnchorObjectSystem.h"
#include "StencilObjectSystem.h"
#include "CompositorScriptContext.h"
#include "ProjectConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorSettings::init(
	Rml::Context* rmlContext,
	ProfileConfigPtr profile)
{
	m_profile = profile;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("render_origin", &m_bRenderOrigin);
	constructor.Bind("render_anchors", &m_bRenderAnchors);
	constructor.Bind("render_stencils", &m_bRenderStencils);
	constructor.Bind("vr_frame_delay", &m_vrFrameDelay);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"update_render_origin_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string stringValue = ev.GetParameter<Rml::String>("value", "");

			m_bRenderOrigin= !stringValue.empty();
			m_profile->setRenderOriginFlag(m_bRenderOrigin);
		});
	constructor.BindEventCallback(
		"update_render_anchors_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string stringValue = ev.GetParameter<Rml::String>("value", "");

			m_bRenderAnchors = !stringValue.empty();
			m_profile->anchorConfig->setRenderAnchorsFlag(m_bRenderAnchors);
		});
	constructor.BindEventCallback(
		"update_render_stencils_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string stringValue = ev.GetParameter<Rml::String>("value", "");

			m_bRenderStencils = !stringValue.empty();
			StencilObjectSystem::getSystem()->setRenderStencilsFlag(m_bRenderStencils);
		});
	constructor.BindEventCallback(
		"update_vr_frame_delay",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_vrFrameDelay = ev.GetParameter<int>("value", 0);
			m_profile->setVRFrameDelay(m_vrFrameDelay);
		});

	m_bRenderOrigin= m_profile->getRenderOriginFlag();
	m_bRenderAnchors= m_profile->anchorConfig->getRenderAnchorsFlag();
	m_bRenderStencils= m_profile->stencilConfig->getRenderStencilsFlag();
	m_vrFrameDelay= m_profile->getVRFrameDelay();
	m_modelHandle.DirtyAllVariables();

	return true;
}

void RmlModel_CompositorSettings::dispose()
{
	m_profile.reset();

	RmlModel::dispose();
}