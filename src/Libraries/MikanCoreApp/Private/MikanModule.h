#pragma once

#include "IMikanModule.h"

#include <type_traits>
#include <string>

class MikanModule
{
public:
	using AllocModuleFunctionPtr = std::add_pointer<IMikanModule* ()>::type;
	using FreeModuleFunctionPtr = std::add_pointer<void(IMikanModule*)>::type;

	MikanModule() = delete;
	MikanModule(const std::string& moduleName);
	~MikanModule();

	inline IMikanModule* getModuleInterface() const
	{
		return m_moduleInterface;
	}

	bool load();
	void unload();

private:
	std::string m_moduleName;
	class dylib* m_module;
	AllocModuleFunctionPtr m_allocModule;
	FreeModuleFunctionPtr m_freeModule;
	IMikanModule* m_moduleInterface;
};