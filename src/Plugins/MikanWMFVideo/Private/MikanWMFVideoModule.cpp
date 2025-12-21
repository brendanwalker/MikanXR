#include "MikanWMFVideoModule.h"
#include "MikanWMFVideoDeviceManager.h"
#include "IUsbVideoDeviceModule.h"

#include "Logger.h"

class MikanWMFVideoModule : public IUsbVideoDeviceModule
{
public:
	MikanWMFVideoModule() : m_bIsInitialized(false)
	{
	}

	virtual ~MikanWMFVideoModule()
	{
		shutdown();
	}

	bool startup() override
	{
		MIKAN_LOG_INFO("MikanWMFVideoModule") << "Initializing MikanWMFVideoModule";
		m_bIsInitialized = true;

		return true;
	}

	void shutdown() override
	{
		// Clean up the WMFVideo library
		if (m_bIsInitialized)
		{
			m_bIsInitialized= false;
		}
	}

	virtual IUsbVideoDeviceManagerPtr createUsbVideoDeviceManager() override
	{
		if (m_bIsInitialized)
		{
			return std::make_shared<MikanWMFVideoDeviceManager>();
		}

		return IUsbVideoDeviceManagerPtr();
	}

private:
	bool m_bIsInitialized;
};

// C-API
IUsbVideoDeviceModule* AllocatePluginModule()
{
	return new MikanWMFVideoModule();
}

void FreePluginModule(IUsbVideoDeviceModule* module)
{
	delete module;
}