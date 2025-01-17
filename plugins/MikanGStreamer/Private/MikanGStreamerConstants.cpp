#include "MikanGStreamerConstants.h"

const char* g_szGStreamerProtocolStrings[(int)eGStreamerProtocol::COUNT] = {
	"rtmp",
	"rtsp"
};
const char** k_szGStreamerProtocolStrings = g_szGStreamerProtocolStrings;