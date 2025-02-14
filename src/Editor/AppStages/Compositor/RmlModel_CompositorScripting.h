#pragma once

#include "ObjectSystemConfigFwd.h"
#include "ScriptingFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

#include <filesystem>
#include <string>

class RmlModel_CompositorScripting : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		ProfileConfigPtr profile,
		CompositorScriptContextPtr scriptContext);
	virtual void dispose() override;

	const std::filesystem::path getCompositorScriptPath() const;
	void setCompositorScriptPath(const std::filesystem::path& newScriptPath);
	void rebuildScriptTriggers();

	SinglecastDelegate<void(const std::filesystem::path&)> OnScriptFileChangeEvent;
	SinglecastDelegate<void()> OnSelectCompositorScriptFileEvent;
	SinglecastDelegate<void()> OnReloadCompositorScriptFileEvent;
	SinglecastDelegate<void(const std::string&)> OnInvokeScriptTriggerEvent;

private:
	ProfileConfigPtr m_profile;
	CompositorScriptContextPtr m_scriptContext;
	Rml::String m_compositorScriptPath;
	bool m_bHasValidCompositorScriptPath;
	Rml::Vector<Rml::String> m_scriptTriggers;
};
