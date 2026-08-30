#include "MikanARKitVideoModule.h"
#include "MikanARKitVideoDeviceManager.h"
#include "IARKitVideoDeviceModule.h"

#include <gst/gst.h>
#include <gst/gstparse.h>

#include "Logger.h"

class MikanARKitVideoModule : public IARKitVideoDeviceModule
{
public:
	MikanARKitVideoModule()
		: m_bIsInitialized(false)
	{
	}

	virtual ~MikanARKitVideoModule() { shutdown(); }

	bool startup() override
	{
		MIKAN_LOG_INFO("MikanARKitVideoModule") << "Initializing MikanARKitVideoModule";

		// Initialize GStreamer (used for the RTP video receive path - see Track C2).
		// Intentionally don't pass in the command line arguments.
		GError* error= nullptr;
		if (gst_init_check(nullptr, nullptr, &error))
		{
			m_bIsInitialized= true;
		}
		else
		{
			MIKAN_LOG_ERROR("MikanARKitVideoModule") << "Failed to init GStreamer" << error->message;
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

	virtual IARKitVideoDeviceManagerPtr createARKitVideoDeviceManager() override
	{
		if (m_bIsInitialized)
		{
			return std::make_shared<MikanARKitVideoDeviceManager>();
		}

		return IARKitVideoDeviceManagerPtr();
	}

private:
	bool m_bIsInitialized;
};

// C-API
IARKitVideoDeviceModule* AllocatePluginModule() { return new MikanARKitVideoModule(); }

void FreePluginModule(IARKitVideoDeviceModule* module) { delete module; }
