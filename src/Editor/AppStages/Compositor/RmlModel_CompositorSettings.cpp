#include "RmlModel_CompositorSettings.h"
#include "AnchorObjectSystem.h"
#include "StencilObjectSystem.h"
#include "CompositorScriptContext.h"
#include "ProjectConfig.h"
#include "RmlUtility.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorSettings::init(
	Rml::Context* rmlContext,
	ProjectConfigPtr project)
{
	m_project = project;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("is_streaming", &m_bIsStreaming);
	constructor.Bind("spout_output_name", &m_spoutOutputName);
	constructor.Bind("render_origin", &m_bRenderOrigin);
	constructor.Bind("render_anchors", &m_bRenderAnchors);
	constructor.Bind("render_stencils", &m_bRenderStencils);
	constructor.Bind("vr_frame_delay", &m_vrFrameDelay);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"update_streaming_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_bIsStreaming= Rml::Utilities::GetBoolValueFromEvent(ev);
			m_project->setIsSpoutOutputStreaming(m_bIsStreaming);
		});
	constructor.BindEventCallback(
		"update_spout_output_name",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (Rml::String spoutOutputName;
				Rml::Utilities::TryGetStringValueFromEvent(ev, spoutOutputName))
			{
				m_project->setIsSpoutOutputStreaming(m_bIsStreaming);
			}
		});
	constructor.BindEventCallback(
		"update_render_origin_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_bRenderOrigin= Rml::Utilities::GetBoolValueFromEvent(ev);
			m_project->setRenderOriginFlag(m_bRenderOrigin);
		});
	constructor.BindEventCallback(
		"update_render_anchors_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_bRenderAnchors = Rml::Utilities::GetBoolValueFromEvent(ev);
			m_project->anchorConfig->setRenderAnchorsFlag(m_bRenderAnchors);
		});
	constructor.BindEventCallback(
		"update_render_stencils_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_bRenderStencils = Rml::Utilities::GetBoolValueFromEvent(ev);
			StencilObjectSystem::getSystem()->setRenderStencilsFlag(m_bRenderStencils);
		});
	constructor.BindEventCallback(
		"update_vr_frame_delay",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_vrFrameDelay = Rml::Utilities::GetIntValueFromEvent(ev);
			m_project->setVRFrameDelay(m_vrFrameDelay);
		});

	m_bIsStreaming = m_project->getIsSpoutOutputStreaming();
	m_spoutOutputName= m_project->getSpoutOutputName();
	m_bRenderOrigin= m_project->getRenderOriginFlag();
	m_bRenderAnchors= m_project->anchorConfig->getRenderAnchorsFlag();
	m_bRenderStencils= m_project->stencilConfig->getRenderStencilsFlag();
	m_vrFrameDelay= m_project->getVRFrameDelay();
	m_modelHandle.DirtyAllVariables();

	return true;
}

void RmlModel_CompositorSettings::dispose()
{
	m_project.reset();

	RmlModel::dispose();
}