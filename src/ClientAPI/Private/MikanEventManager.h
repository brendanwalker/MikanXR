#pragma once

#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"

class MikanEventManager
{
public:
	MikanEventManager() = default;

	MikanResult fetchNextEvent(MikanEventPtr& out_event);

protected:
	MikanEventPtr parseEventString(const char* utf8EventString);
};