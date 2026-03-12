#pragma once

#include <stdint.h>

class EventBus;

// The id of an event bus subscription
using EventBusSubID = int32_t;

// Invalid subscription id value
#define INVALID_EVENTBUS_SUB_ID -1