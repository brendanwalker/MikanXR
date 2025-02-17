#pragma once

#include "MikanAPITypes.h"
#include "MikanTypeFwd.h"

typedef void* MikanContext;

class MikanEventManager
{
public:
	MikanEventManager() = default;

	MikanAPIResult init(MikanContext context);
	MikanAPIResult fetchNextEvent(MikanEventPtr& out_event);

protected:
	MikanEventPtr parseEventString(const char* utf8EventString);

private:
	MikanContext m_context = nullptr;
};