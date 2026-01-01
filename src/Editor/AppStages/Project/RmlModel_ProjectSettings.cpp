#include "RmlModel_ProjectSettings.h"
#include "AnchorObjectSystem.h"
#include "LocalizationManager.h"
#include "StencilObjectSystem.h"
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
	m_stencilSystem = ownerAppStage->getObjectSystemOfType<StencilObjectSystem>();

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
			bool value = m_anchorSystem.lock()->getAnchorSystemConfig()->getRenderAnchorsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_anchorSystem.lock()->getAnchorSystemConfig()->setRenderAnchorsFlag(value);
		});

	constructor.BindFunc(
		"render_stencils",
		[this](Rml::Variant& variant) {
			bool value= m_stencilSystem.lock()->getStencilSystemConfig()->getRenderStencilsFlag();
			variant = Rml::Variant(value);
		},
		[this](const Rml::Variant& variant) {
			bool value = variant.Get<bool>();
			m_stencilSystem.lock()->getStencilSystemConfig()->setRenderStencilsFlag(value);
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