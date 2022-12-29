#include "RmlModel_CompositorScripting.h"
#include "ProfileConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>


bool RmlModel_CompositorScripting::init(
	Rml::Context* rmlContext,
	const ProfileConfig* profile)
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
			const int listIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnInvokeScriptTriggerEvent && listIndex >= 0 && listIndex < (int)m_scriptTriggers.size()){
				OnInvokeScriptTriggerEvent(m_scriptTriggers[listIndex]);
			}
		});	

	// Set defaults
	setCompositorScriptPath(profile->compositorScript);

	return true;
}

void RmlModel_CompositorScripting::dispose()
{
	OnSelectCompositorScriptFileEvent.Clear();
	OnReloadCompositorScriptFileEvent.Clear();
	OnInvokeScriptTriggerEvent.Clear();
	RmlModel::dispose();
}

const Rml::String& RmlModel_CompositorScripting::getCompositorScriptPath() const
{
	return m_compositorScriptPath;
}

void RmlModel_CompositorScripting::setCompositorScriptPath(const Rml::String& newScriptPath)
{
	if (newScriptPath != m_compositorScriptPath)
	{
		m_compositorScriptPath= newScriptPath;
		m_modelHandle.DirtyVariable("");
		
		bool bNewValidPath = m_compositorScriptPath.size() > 0;
		if (bNewValidPath != m_bHasValidCompositorScriptPath)
		{
			m_bHasValidCompositorScriptPath = bNewValidPath;
			m_modelHandle.DirtyVariable("has_valid_compositor_script_path");
		}
	}
}

const Rml::Vector<Rml::String>& RmlModel_CompositorScripting::getScriptTriggers() const
{
	return m_scriptTriggers;
}

void RmlModel_CompositorScripting::setScriptTriggers(const Rml::Vector<Rml::String>& newTriggers)
{
	m_scriptTriggers= newTriggers;
	m_modelHandle.DirtyVariable("script_triggers");
}