#include "MikanModuleManager.h"
#include "MikanModule.h"

#include <map>
#include <memory>

#include "assert.h"

class MikanModuleManager : public IMikanModuleManager
{
public:
	virtual ~MikanModuleManager() 
	{
		shutdown();
	};

	bool init()
	{
		return true;
	}

	void shutdown()
	{
		// Unload all modules
		for (auto& module : m_modules)
		{
			delete module.second;
		}
		m_modules.clear();
	}

	virtual IMikanModule* getModule(const std::string& moduleName) override
	{
		// Return the module if it's already loaded
		auto it = m_modules.find(moduleName);
		if (it != m_modules.end())
		{
			return it->second->getModuleInterface();
		}

		// Attempt to load the module
		MikanModule* module = new MikanModule(moduleName);
		if (module->load())
		{
			m_modules.insert({moduleName, module});

			return module->getModuleInterface();
		}

		// Return nullptr if the module failed to load
		return nullptr;	
	}

	virtual bool disposeModule(IMikanModule* moduleIterface) override
	{
		for (auto it = m_modules.begin(); it != m_modules.end(); it++)
		{
			MikanModule* module= it->second;

			if (module->getModuleInterface() == moduleIterface)
			{
				// Remove the module entry from the map
				m_modules.erase(it);

				// Unload the module
				module->unload();

				// Free the module entry object
				delete module;

				return true;
			}
		}

		return false;
	}

private:
	std::map<std::string, MikanModule*> m_modules;
};

static MikanModuleManager* g_moduleManager = nullptr;

bool initMikanModuleManager()
{
	if (g_moduleManager == nullptr)
	{
		g_moduleManager = new MikanModuleManager();

		if (!g_moduleManager->init())
		{
			delete g_moduleManager;
			g_moduleManager = nullptr;
		}
	}

	return g_moduleManager != nullptr;
}

void shutdownMikanModuleManager()
{
	if (g_moduleManager != nullptr)
	{
		delete g_moduleManager;
		g_moduleManager = nullptr;
	}
}

IMikanModuleManager* getMikanModuleManager()
{
	assert(g_moduleManager != nullptr);
	return g_moduleManager;
}