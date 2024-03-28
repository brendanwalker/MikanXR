#pragma once

#include "MikanAPITypes.h"

#include "nlohmann/json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanClientInfo,
								   clientId,
								   engineName,
								   engineVersion,
								   applicationName,
								   applicationVersion,
								   xrDeviceName,
								   graphicsAPI,
								   mikanCoreSdkVersion,
								   supportsRBG24,
								   supportsRBGA32,
								   supportsBGRA32,
								   supportsDepth)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanColorRGB,
									r,
									g,
									b)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanResponse,
									responseType,
									requestId,
									resultCode)


