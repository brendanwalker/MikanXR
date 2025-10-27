#pragma once

#include "ComponentScriptContext.h"

//-- definitions -----
class SceneComponentScriptContext : public ComponentScriptContext
{
public:
	SceneComponentScriptContext() = default;
	virtual ~SceneComponentScriptContext() {};

protected:
	virtual bool bindContextFunctions() override;
};