#include "RmlModel_CompositorScripting.h"
#include "CompositorScriptContext.h"
#include "ProfileConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>


bool RmlModel_CompositorScripting::init(
	Rml::Context* rmlContext,
	const ProfileConfig* profile,
	CompositorScriptContext* scriptContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_scripting");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("compositor_script_path", &m_compositorScriptPath);
	constructor.Bind("has_valid_compositor_script_path", &m_bHasValidCompositorScriptPath);
	constructor.Bind("script_triggers", &m_scriptTriggers);	

	// Bind data model callbacks
	constructor.BindEventCallback(
		"edit_compositor_script_file",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnScriptFileChangeEvent && ev.GetId() == Rml::EventId::Change)
			{
				const bool isLineBreak = ev.GetParameter("linebreak", false);
				const std::filesystem::path newScriptFile = ev.GetParameter<Rml::String>("value", "");
				if (isLineBreak && !newScriptFile.empty())
				{
					OnScriptFileChangeEvent(newScriptFile);
				}
			}
		});
	constructor.BindEventCallback(
		"select_compositor_script_file",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnSelectCompositorScriptFileEvent) OnSelectCompositorScriptFileEvent();
		});
	constructor.BindEventCallback(
		"reload_compositor_script_file",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnReloadCompositorScriptFileEvent) OnReloadCompositorScriptFileEvent();
		});
	constructor.BindEventCallback(
		"invoke_script_trigger",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
		const Rml::String scriptTrigger = (arguments.size() == 1 ? arguments[0].Get<Rml::String>("") : "");
			if (OnInvokeScriptTriggerEvent && !scriptTrigger.empty()){
				OnInvokeScriptTriggerEvent(scriptTrigger);
			}
		});	

	// Set defaults
	setCompositorScriptPath(profile->compositorScriptFilePath.string());
	rebuildScriptTriggers(scriptContext);

	return true;
}

void RmlModel_CompositorScripting::dispose()
{
	OnSelectCompositorScriptFileEvent.Clear();
	OnReloadCompositorScriptFileEvent.Clear();
	OnInvokeScriptTriggerEvent.Clear();
	RmlModel::dispose();
}

const std::filesystem::path RmlModel_CompositorScripting::getCompositorScriptPath() const
{
	return m_compositorScriptPath;
}

void RmlModel_CompositorScripting::setCompositorScriptPath(
	const std::filesystem::path& newScriptPath)
{
	const std::string newScriptPathString = newScriptPath.string();

	if (newScriptPathString != m_compositorScriptPath)
	{
		m_compositorScriptPath= newScriptPathString;
		m_modelHandle.DirtyVariable("compositor_script_path");
		
		bool bNewValidPath = m_compositorScriptPath.size() > 0;
		if (bNewValidPath != m_bHasValidCompositorScriptPath)
		{
			m_bHasValidCompositorScriptPath = bNewValidPath;
			m_modelHandle.DirtyVariable("has_valid_compositor_script_path");
		}
	}
}

void RmlModel_CompositorScripting::rebuildScriptTriggers(CompositorScriptContext* scriptContext)
{
	const std::vector<std::string>& sourceTriggers= scriptContext->getScriptTriggers();
	m_scriptTriggers.clear();
	for (const std::string& scriptTrigger : sourceTriggers)
	{
		m_scriptTriggers.push_back(scriptTrigger);
	}
	m_modelHandle.DirtyVariable("script_triggers");
}