#include "RmlModel_ProjectSettings.h"
#include "AnchorObjectSystem.h"
#include "LocalizationManager.h"
#include "QuadStencilSystem.h"
#include "BoxStencilSystem.h"
#include "ModelStencilSystem.h"
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
	auto* localizationManager = LocalizationManager::getInstance();
	m_selectedLangugeId = localizationManager->getLanguage();
	m_languageIdList = localizationManager->getSupportedLanguages();

	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	Rml::Context* rmlContext = ownerAppStage->getRmlContext();

	m_projectRmlModelContext = context;
	m_project = ownerAppStage->getProjectConfig();
	m_anchorSystem = ownerAppStage->getObjectSystemOfType<AnchorObjectSystem>();
	m_quadStencilSystem = ownerAppStage->getObjectSystemOfType<QuadStencilSystem>();
	m_boxStencilSystem = ownerAppStage->getObjectSystemOfType<BoxStencilSystem>();
	m_modelStencilSystem = ownerAppStage->getObjectSystemOfType<ModelStencilSystem>();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "Settings");
	if (!constructor)
		return false;

	constructor.BindFunc(
		"render_origin",
		[this](Rml::Variant& variant) {
			bool value = m_project.lock()->getRenderOriginFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_project.lock()->setRenderOriginFlag(value);
		});

	constructor.BindFunc(
		"render_anchors",
		[this](Rml::Variant& variant) {
			bool value = m_anchorSystem.lock()->getTypedDefinition()->getRenderAnchorsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_anchorSystem.lock()->getTypedDefinition()->setRenderAnchorsFlag(value);
		});

	constructor.BindFunc(
		"render_quad_stencils",
		[this](Rml::Variant& variant) {
			// Get the flag from any stencil system (they should all be in sync)
			QuadStencilSystemPtr quadStencilSystem = m_quadStencilSystem.lock();
			bool value= quadStencilSystem->getTypedDefinitionConst()->getRenderStencilsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_quadStencilSystem.lock()->getTypedDefinition()->setRenderStencilsFlag(value);
		});

	constructor.BindFunc(
		"render_box_stencils",
		[this](Rml::Variant& variant) {
			// Get the flag from any stencil system (they should all be in sync)
			BoxStencilSystemPtr boxStencilSystem = m_boxStencilSystem.lock();
			bool value = boxStencilSystem->getTypedDefinitionConst()->getRenderStencilsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_boxStencilSystem.lock()->getTypedDefinition()->setRenderStencilsFlag(value);
		});

	constructor.BindFunc(
		"render_model_stencils",
		[this](Rml::Variant& variant) {
			// Get the flag from any stencil system (they should all be in sync)
			ModelStencilSystemPtr modelStencilSystem = m_modelStencilSystem.lock();
			bool value = modelStencilSystem->getTypedDefinitionConst()->getRenderStencilsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_modelStencilSystem.lock()->getTypedDefinition()->setRenderStencilsFlag(value);
		});


	constructor.Bind("language_id_list", &m_languageIdList);
	constructor.Bind("selected_language_id", &m_selectedLangugeId);
	constructor.BindEventCallback(
		"select_language_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string newLanguage = ev.GetParameter<std::string>("value", "");
			LocalizationManager::getInstance()->setLanguage(newLanguage);
		});

	m_modelHandle.DirtyAllVariables();

	return true;
}