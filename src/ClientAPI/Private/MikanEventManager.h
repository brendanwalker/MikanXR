#pragma once

#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"

typedef void* MikanContext;

class MikanEventManager
{
public:
	MikanEventManager() = default;

	MikanResult init(MikanContext context);
	MikanResult fetchNextEvent(MikanEventPtr& out_event);

protected:
	MikanEventPtr parseEventString(const char* utf8EventString);

private:
	MikanContext m_context = nullptr;
};