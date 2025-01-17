#include "MikanGStreamerModule.h"
#include "MikanGStreamerVideo.h"

//#include "Logger.h"

#include <gst/gst.h>
#include <gst/gstparse.h>

class MikanGStreamerModule : public IMikanGStreamerModule
{
public:
	MikanGStreamerModule() : m_bIsInitialized(false)
	{
	}

	virtual ~MikanGStreamerModule()
	{
		shutdown();
	}

	bool startup() override
	{
		// Initialize GStreamer
		// Intentionally don't pass in the command line arguments
		GError* error = nullptr;
		if (gst_init_check(nullptr, nullptr, &error))
		{
			m_bIsInitialized = true;
		}
		else
		{
			//MIKAN_LOG_ERROR("MikanGStreamerModule") << "Failed to init GStreamer" << error->message;
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

	MikanGStreamerVideoDevicePtr createVideoDevice(const MikanGStreamerSettings& settings) override
	{
		if (m_bIsInitialized)
		{
			return std::make_shared<MikanGStreamerVideoDevice>(settings);
		}

		return MikanGStreamerVideoDevicePtr();
	}

private:
	bool m_bIsInitialized;
};

// C-API
IMikanGStreamerModule* AllocateModule()
{
	return new MikanGStreamerModule();
}

void FreeModule(IMikanGStreamerModule* module)
{
	delete module;
}