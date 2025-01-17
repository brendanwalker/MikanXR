#pragma once

#include "MikanGStreamerExport.h"

enum class eGStreamerProtocol
{
	INVALID = -1,

	RTMP,
	RTSP,

	COUNT
};
MIKAN_GSTREAMER_API extern const char** k_szGStreamerProtocolStrings;