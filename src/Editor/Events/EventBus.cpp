#include "EventBus.h"

EventBus::EventBus()
	: m_impl(std::make_unique<EventBusImpl>())
{
}

EventBus::~EventBus() {}
