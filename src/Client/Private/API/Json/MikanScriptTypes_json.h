#pragma once

#include "MikanScriptTypes.h"

#include "nlohmann/json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanScriptMessageInfo,
								   content
)