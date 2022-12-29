#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_CompositorScripting : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class ProfileConfig* profile);
	virtual void dispose() override;

	const Rml::String& getCompositorScriptPath() const;
	void setCompositorScriptPath(const Rml::String& newScriptPath);

	const Rml::Vector<Rml::String>& getScriptTriggers() const;
	void setScriptTriggers(const Rml::Vector<Rml::String>& newTriggers);

	SinglecastDelegate<void()> OnSelectCompositorScriptFileEvent;
	SinglecastDelegate<void()> OnReloadCompositorScriptFileEvent;
	SinglecastDelegate<void(const std::string&)> OnInvokeScriptTriggerEvent;

private:
	Rml::String m_compositorScriptPath;
	bool m_bHasValidCompositorScriptPath;
	Rml::Vector<Rml::String> m_scriptTriggers;
};
