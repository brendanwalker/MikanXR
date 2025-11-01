#pragma once

#include "ComponentScriptContext.h"

//-- definitions -----
class SceneComponentScriptContext : public ComponentScriptContext
{
public:
	SceneComponentScriptContext(MikanComponentPtr ownerComponent);
	virtual ~SceneComponentScriptContext() {};

protected:
	virtual bool bindContextFunctions() override;
};