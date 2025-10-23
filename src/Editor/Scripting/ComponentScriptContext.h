#pragma once

#include "CommonScriptContext.h"

//-- definitions -----
class ComponentScriptContext : public CommonScriptContext
{
public:
	ComponentScriptContext();
	virtual ~ComponentScriptContext();

protected:
	virtual bool bindContextFunctions() override;
	void bindStencilFunctions();
};