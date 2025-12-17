#include "AppSettingsConfig.h"
#include "RmlModel_MainMenu.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_MainMenu::init(
	Rml::Context* rmlContext,
	AppSettingsConfigPtr appSettings)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "MainMenu");
	if (!constructor)
		return false;

	m_appSettingsConfig = appSettings;

	constructor.BindFunc(
		"has_existing_project",
		[this](Rml::Variant& variant) {
			bool value = m_appSettingsConfig.lock()->hasLastProjectPath();
			variant = Rml::Variant(value);
		});

	constructor.BindEventCallback(
		"resume_project",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnResumeProject) OnResumeProject();
		});
	constructor.BindEventCallback(
		"open_project",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnOpenProject) OnOpenProject();
		});
	constructor.BindEventCallback(
		"new_project",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnNewProject) OnNewProject();
		});
	constructor.BindEventCallback(
		"launch_tutorial",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnTutorial) OnTutorial();
		});
	constructor.BindEventCallback(
		"exit",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnExit) OnExit();
		});


	constructor.Bind("language_id_list", &m_languageIdList);
	constructor.Bind("selected_language_id", &m_selectedLangugeId);
	constructor.BindEventCallback(
		"select_language_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string newLanguage = ev.GetParameter<std::string>("value", "");
			if (m_appSettingsConfig.lock()) {
				m_appSettingsConfig.lock()->setAppLanguage(newLanguage);
			}
		});

	m_modelHandle.DirtyAllVariables();

	return true;
}

void RmlModel_MainMenu::dispose()
{
	OnResumeProject.Clear();
	OnOpenProject.Clear();
	OnNewProject.Clear();
	OnTutorial.Clear();
	OnExit.Clear();

	RmlModel::dispose();
}