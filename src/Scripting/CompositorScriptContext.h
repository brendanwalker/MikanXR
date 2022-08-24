#pragma once

#include "CommonScriptContext.h"

//-- definitions -----
class CompositorScriptContext : public CommonScriptContext
{
public:
	CompositorScriptContext();
	virtual ~CompositorScriptContext();

protected:
	virtual bool bindContextFunctions() override;
	void bindStencilFunctions();
};