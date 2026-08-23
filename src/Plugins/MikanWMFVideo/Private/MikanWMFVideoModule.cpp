#include "MikanWMFVideoModule.h"
#include "MikanWMFVideoDeviceManager.h"
#include "IUsbVideoDeviceModule.h"

#include "Logger.h"

#include <Mfidl.h>
#include <Mfapi.h>
#include <Mferror.h>

class MikanWMFVideoModule : public IUsbVideoDeviceModule
{
public:
	MikanWMFVideoModule()
		: m_bIsInitialized(false)
	{
	}

	virtual ~MikanWMFVideoModule() { shutdown(); }

	bool startup() override
	{
		if (!m_bIsInitialized)
		{

			HRESULT hr= MFStartup(MF_VERSION);
			if (SUCCEEDED(hr))
			{
				MIKAN_LOG_INFO("MikanWMFVideoModule") << "Successfully initialized Media Foundation";
				m_bIsInitialized= true;
			}
			else
			{
				MIKAN_LOG_ERROR("MikanWMFVideoModule") << "Failed to initialize Media Foundation.";
			}
		}

		return m_bIsInitialized;
	}

	void shutdown() override
	{
		// Clean up the WMFVideo library
		if (m_bIsInitialized)
		{
			MFShutdown();
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
IUsbVideoDeviceModule* AllocatePluginModule() { return new MikanWMFVideoModule(); }

void FreePluginModule(IUsbVideoDeviceModule* module) { delete module; }