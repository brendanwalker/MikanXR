#pragma once

#include "CommonScriptContext.h"
#include "ComponentFwd.h"

//-- definitions -----
class ComponentScriptContext : public CommonScriptContext
{
public:
	ComponentScriptContext(MikanComponentPtr ownerComponent);
	virtual ~ComponentScriptContext() {}

	MikanComponentPtr getOwnerComponent() const { return m_ownerComponent.lock(); }

protected:
	virtual bool bindContextFunctions() override;

private:
	MikanComponentWeakPtr m_ownerComponent;
};