#include "MikanModule.h"
#include "Logger.h"
#include "PathUtils.h"

#include <cstdlib>
#include <fstream>

#include "dylib.hpp"

MikanModule::MikanModule(const std::string& moduleName)
	: m_moduleName(moduleName)
	, m_module(nullptr)
	, m_allocModule(nullptr)
	, m_freeModule(nullptr)
	, m_moduleInterface(nullptr)
{}

MikanModule::~MikanModule()
{
	unload();
}

bool MikanModule::load()
{
	bool success = false;

	try
	{
		// Assume plugin module is in the same place as the executable
		std::string modulePath = PathUtils::getModulePath().string();

		// Load the GStreamer library
		m_module = new dylib(modulePath.c_str(), m_moduleName);

		// Fetch the functions we need
		m_allocModule = m_module->get_function<IMikanModule * ()>(ALLOCATE_MODULE_FUNCTION_NAME);
		m_freeModule = m_module->get_function<void(IMikanModule*)>(FREE_MODULE_FUNCTION_NAME);

		// Attempt to allocate the module
		m_moduleInterface = m_allocModule();

		// Attempt to initiate the module
		if (m_moduleInterface != nullptr)
		{
			success = m_moduleInterface->startup();
			if (!success)
			{
				MIKAN_LOG_ERROR("MikanModule::startup") << "Failed to initialize " << m_moduleName << " module!";
			}
		}
	}
	catch (std::exception* e)
	{
		MIKAN_LOG_ERROR("MikanModule::startup") << "Failed to load " << m_moduleName << " module: " << e->what();
		return false;
	}

	if (!success)
	{
		unload();
	}

	return success;
}

void MikanModule::unload()
{
	if (m_module != nullptr)
	{
		if (m_freeModule != nullptr)
		{
			if (m_moduleInterface != nullptr)
			{
				m_freeModule(m_moduleInterface);
			}
		}

		delete m_module;
	}

	m_module = nullptr;
	m_allocModule = nullptr;
	m_freeModule = nullptr;
	m_moduleInterface = nullptr;
}