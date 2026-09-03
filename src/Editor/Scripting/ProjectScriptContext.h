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

private:
	ProjectManagerWeakPtr m_projectManager;
};
