#pragma once

#include "IMikanModule.h"
#include "MikanGStreamerVideoInterface.h"

class IMikanGStreamerModule : public IMikanModule
{
public:
	IMikanGStreamerModule() = default;

	virtual bool startup() = 0;
	virtual void shutdown() = 0;

	virtual MikanGStreamerVideoDevicePtr createVideoDevice(const MikanGStreamerSettings& settings) = 0;
};

MIKAN_GSTREAMER_FUNC(IMikanGStreamerModule*) AllocatePluginModule();
MIKAN_GSTREAMER_FUNC(void) FreePluginModule(IMikanGStreamerModule* module);