#pragma once

#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"

#include <functional>
#include <memory>
#include <map>
#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class MikanEventManager
{
public:
	MikanEventManager() = default;

	template <typename t_event_type>
	void addEventFactory()
	{
		MikanEventFactory factory =
			[](json jsonEvent) -> MikanEventPtr {
			auto eventPtr = std::make_shared<t_event_type>();

			*eventPtr = jsonEvent.get<t_event_type>();

			return eventPtr;
		};

		m_eventFactories.insert(std::make_pair(t_event_type::k_typeName, factory));
	}

	MikanResult fetchNextEvent(MikanEventPtr& out_event);

protected:
	MikanEventPtr parseEventString(const char* utf8EventString);

private:
	std::map<std::string, MikanEventFactory> m_eventFactories;
};