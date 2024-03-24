#pragma once

#include "MikanClientTypes.h"
#include "Logger.h"

#include "MikanClientTypes_json.h"
#include "MikanEventTypes_json.h"

#include <functional>
#include <memory>
#include <map>
#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;
using MikanEventPtr = std::shared_ptr<MikanEvent>;
using MikanEventFactory = std::function<MikanEventPtr(const std::string& eventType)>;

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

	MikanResult fetchNextEvent(MikanEventPtr& out_event)
	{
		char utf8Buffer[1024];
		size_t utf8BytesWritten = 0;

		MikanResult result = Mikan_FetchNextEvent(sizeof(utf8Buffer), utf8Buffer, &utf8BytesWritten);
		if (result == MikanResult_Success)
		{
			out_event = parseEventString(utf8Buffer);
		}

		return result;
	}

protected:
	MikanEventPtr parseEventString(const char* utf8EventString)
	{
		MikanEventPtr eventPtr;

		try
		{
			json jsonResponse = json::parse(utf8EventString);
			std::string eventType = jsonResponse["eventType"].get<std::string>();
			auto it = m_eventFactories.find(eventType);

			if (it != m_eventFactories.end())
			{
				MikanEventFactory factory = it->second;

				eventPtr = factory(jsonResponse);
			}
			else
			{
				MIKAN_MT_LOG_WARNING("MikanClient::parseEventString()")
					<< "Received response for unknown eventType: " << eventType;
			}
		}
		catch (json::parse_error& e)
		{
			MIKAN_MT_LOG_ERROR("MikanClient::parseEventString()")
				<< "Failed to parse event: " << e.what();
		}
		catch (json::exception& e)
		{
			MIKAN_MT_LOG_ERROR("MikanClient::parseEventString()")
				<< "Failed to parse event: " << e.what();
		}

		return eventPtr;
	}

private:
	std::map<std::string, MikanEventFactory> m_eventFactories;
};