#include "RmlModel_ProjectSettings.h"
#include "EditorObjectSystem.h"
#include "LocalizationManager.h"
#include "StencilUtils.h"
#include "ProjectConfig.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectRmlModelContext.h"
#include "RmlUtility.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_ProjectSettings::init(ProjectRmlModelContext* context)
{
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	Rml::Context* rmlContext = ownerAppStage->getRmlContext();

	m_projectRmlModelContext = context;
	m_project = ownerAppStage->getProjectConfig();
	m_editorSystem = ownerAppStage->getObjectSystemOfType<EditorObjectSystem>();
	m_localizationManager = ownerAppStage->getOwnerWindow()->getLocalizationManager();
	m_selectedLangugeId = m_localizationManager->getLanguage();
	m_languageIdList = m_localizationManager->getSupportedLanguages();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "Settings");
	if (!constructor)
		return false;

	constructor.BindFunc(
		"render_origin",
		[this](Rml::Variant& variant) {
			bool value = m_editorSystem.lock()->getEditorSystemConfigConst()->getRenderOriginFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderOriginFlag(value);
		});

	constructor.BindFunc(
		"render_anchors",
		[this](Rml::Variant& variant) {
			bool value = m_editorSystem.lock()->getEditorSystemConfigConst()->getRenderAnchorsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderAnchorsFlag(value);
		});

	constructor.BindFunc(
		"render_quad_stencils",
		[this](Rml::Variant& variant) {
			// Get the flag from any stencil system (they should all be in sync)
			bool value= m_editorSystem.lock()->getEditorSystemConfigConst()->getRenderQuadStencilsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderQuadStencilsFlag(value);
		});

	constructor.BindFunc(
		"render_box_stencils",
		[this](Rml::Variant& variant) {
			// Get the flag from any stencil system (they should all be in sync)
			bool value= m_editorSystem.lock()->getEditorSystemConfigConst()->getRenderBoxStencilsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderBoxStencilsFlag(value);
		});

	constructor.BindFunc(
		"render_model_stencils",
		[this](Rml::Variant& variant) {
			// Get the flag from any stencil system (they should all be in sync)
			bool value= m_editorSystem.lock()->getEditorSystemConfigConst()->getRenderModelStencilsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderModelStencilsFlag(value);
		});


	constructor.Bind("language_id_list", &m_languageIdList);
	constructor.Bind("selected_language_id", &m_selectedLangugeId);
	constructor.BindEventCallback(
		"select_language_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string newLanguage = ev.GetParameter<std::string>("value", "");
			m_localizationManager->setLanguage(newLanguage);
		});

	m_modelHandle.DirtyAllVariables();

	return true;
}