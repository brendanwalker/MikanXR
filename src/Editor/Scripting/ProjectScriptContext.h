#pragma once

#include "CommonScriptContext.h"
#include "ObjectSystemFwd.h"

//-- definitions -----
// The project's one script context: binds the object system and component
// classes and exposes each scriptable object system as a global
class ProjectScriptContext : public CommonScriptContext
{
public:
	ProjectScriptContext(ProjectManagerPtr projectManager);
	virtual ~ProjectScriptContext() {}

protected:
	virtual bool bindContextFunctions() override;
	// The project's component of that id, only when it is of the named class
	virtual MikanComponentPtr resolveComponent(const std::string& componentClass,
											   MikanComponentID componentId) const override;

private:
	ProjectManagerWeakPtr m_projectManager;
};
