#pragma once

#include "MikanGStreamerVideoInterface.h"

class IMikanGStreamerModule
{
public:
	IMikanGStreamerModule() = default;
	virtual ~IMikanGStreamerModule() = default;

	virtual bool startup() = 0;
	virtual void shutdown() = 0;

	virtual MikanGStreamerVideoDevicePtr createVideoDevice(const MikanGStreamerSettings& settings) = 0;
};

// C-API
//#define TEST_GSTREAMER_CAPI(rval)  rval
MIKAN_GSTREAMER_CAPI(IMikanGStreamerModule*) AllocatePluginModule();
MIKAN_GSTREAMER_CAPI(void) FreePluginModule(IMikanGStreamerModule* module);