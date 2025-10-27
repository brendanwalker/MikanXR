#include "ComponentScriptContext.h"

bool ComponentScriptContext::bindContextFunctions()
{
	if (!CommonScriptContext::bindContextFunctions())
		return false;

	//TODO: Bind component-specific functions here

	return true;
}