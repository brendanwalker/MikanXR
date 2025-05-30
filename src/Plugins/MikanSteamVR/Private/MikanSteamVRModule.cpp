#include "MikanSteamVRModule.h"
#include "MikanSteamVRManager.h"
#include "Logger.h"

class MikanSteamVRModule : public IVRDeviceModule
{
public:
	MikanSteamVRModule() : m_bIsInitialized(false)
	{
	}

	virtual ~MikanSteamVRModule()
	{
		shutdown();
	}

	bool startup() override
	{
		MIKAN_LOG_INFO("MikanSteamVRModule") << "Initializing MikanSteamVRPlugin";

		return m_bIsInitialized;
	}

	void shutdown() override
	{
		// Clean up the GStreamer library
		if (m_bIsInitialized)
		{
			m_bIsInitialized= false;
		}
	}

	IVRDeviceManagerPtr createVRDeviceManager() override
	{
		if (m_bIsInitialized)
		{
			return std::make_shared<MikanSteamVRManager>();
		}

		return IVRDeviceManagerPtr();
	}

private:
	bool m_bIsInitialized;
};

// C-API
IVRDeviceModule* AllocatePluginModule()
{
	return new MikanSteamVRModule();
}

void FreePluginModule(IVRDeviceModule* module)
{
	delete module;
}