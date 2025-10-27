#pragma once

#include "CommonScriptContext.h"

//-- definitions -----
class ComponentScriptContext : public CommonScriptContext
{
public:
	ComponentScriptContext() = default;
	virtual ~ComponentScriptContext() {}

protected:
	virtual bool bindContextFunctions() override;
};