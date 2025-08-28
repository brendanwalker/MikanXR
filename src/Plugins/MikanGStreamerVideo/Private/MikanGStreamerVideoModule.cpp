#include "MikanGStreamerVideoModule.h"
#include "MikanGStreamerVideoDeviceManager.h"
#include "INetworkVideoDeviceModule.h"

#include <gst/gst.h>
#include <gst/gstparse.h>

#include "Logger.h"

class MikanGStreamerVideoModule : public INetworkVideoDeviceModule
{
public:
	MikanGStreamerVideoModule() : m_bIsInitialized(false)
	{
	}

	virtual ~MikanGStreamerVideoModule()
	{
		shutdown();
	}

	bool startup() override
	{
		MIKAN_LOG_INFO("MikanNetworkVideoModule") << "Initializing MikanNetworkVideoModule";

		// Initialize GStreamer
		// Intentionally don't pass in the command line arguments
		GError* error = nullptr;
		if (gst_init_check(nullptr, nullptr, &error))
		{
			m_bIsInitialized = true;
		}
		else
		{
			MIKAN_LOG_ERROR("MikanGStreamerModule") << "Failed to init GStreamer" << error->message;
			gst_deinit();
		}

		return m_bIsInitialized;
	}

	void shutdown() override
	{
		// Clean up the GStreamer library
		if (m_bIsInitialized)
		{
			gst_deinit();
			m_bIsInitialized= false;
		}
	}

	virtual INetworkVideoDeviceManagerPtr createNetworkVideoDeviceManager() override
	{
		if (m_bIsInitialized)
		{
			return std::make_shared<MikanGStreamerVideoDeviceManager>();
		}

		return INetworkVideoDeviceManagerPtr();
	}

private:
	bool m_bIsInitialized;
};

// C-API
INetworkVideoDeviceModule* AllocatePluginModule()
{
	return new MikanGStreamerVideoModule();
}

void FreePluginModule(INetworkVideoDeviceModule* module)
{
	delete module;
}