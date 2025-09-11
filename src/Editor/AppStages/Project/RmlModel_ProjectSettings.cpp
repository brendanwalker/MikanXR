#include "RmlModel_ProjectSettings.h"
#include "AnchorObjectSystem.h"
#include "StencilObjectSystem.h"
#include "ProjectConfig.h"
#include "RmlUtility.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_ProjectSettings::init(
	Rml::Context* rmlContext,
	ProjectConfigPtr project,
	StencilObjectSystemPtr stencilSystem)
{
	m_project = project;
	m_stencilSystem = stencilSystem;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("render_origin", &m_bRenderOrigin);
	constructor.Bind("render_anchors", &m_bRenderAnchors);
	constructor.Bind("render_stencils", &m_bRenderStencils);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"update_render_origin_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_bRenderOrigin= Rml::Utilities::GetBoolValueFromEvent(ev);
			m_project.lock()->setRenderOriginFlag(m_bRenderOrigin);
		});
	constructor.BindEventCallback(
		"update_render_anchors_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_bRenderAnchors = Rml::Utilities::GetBoolValueFromEvent(ev);
			m_project.lock()->anchorConfig->setRenderAnchorsFlag(m_bRenderAnchors);
		});
	constructor.BindEventCallback(
		"update_render_stencils_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_bRenderStencils = Rml::Utilities::GetBoolValueFromEvent(ev);
			m_stencilSystem.lock()->setRenderStencilsFlag(m_bRenderStencils);
		});

	m_bRenderOrigin= project->getRenderOriginFlag();
	m_bRenderAnchors= project->anchorConfig->getRenderAnchorsFlag();
	m_bRenderStencils= project->stencilConfig->getRenderStencilsFlag();
	m_modelHandle.DirtyAllVariables();

	return true;
}

void RmlModel_ProjectSettings::dispose()
{
	m_project.reset();

	RmlModel::dispose();
}