#include "MikanEventManager.h"
#include "MikanAPITypes.h"
#include "MikanCoreCAPI.h"
#include "MikanClientEvents.h"
#include "Logger.h"
#include "JsonDeserializer.h"

#include <Refureku/Refureku.h>
#include <string>

#include "nlohmann/json.hpp"

#define WEBSOCKET_CONNECT_EVENT				"connect"
#define WEBSOCKET_DISCONNECT_EVENT			"disconnect"
#define WEBSOCKET_ERROR_EVENT				"error"
#define WEBSOCKET_PING_EVENT				"ping"
#define WEBSOCKET_PONG_EVENT				"pong"

using json = nlohmann::json;

MikanResult MikanEventManager::init(MikanContext context)
{
	m_context = context;

	return MikanResult_Success;
}

MikanResult MikanEventManager::fetchNextEvent(MikanEventPtr& out_event)
{
	char utf8Buffer[1024];
	size_t utf8BytesWritten = 0;

	MikanResult result = Mikan_FetchNextEvent(m_context, sizeof(utf8Buffer), utf8Buffer, &utf8BytesWritten);
	if (result == MikanResult_Success)
	{
		out_event = parseEventString(utf8Buffer);
		if (!out_event)
		{
			MIKAN_MT_LOG_WARNING("MikanClient::fetchNextEvent()")
				<< "Failed to parse event string: " << utf8Buffer;
			result = MikanResult_MalformedResponse;
		}
	}

	return result;
}

MikanEventPtr MikanEventManager::parseEventString(const char* szUtf8EventString)
{
	MikanEventPtr eventPtr;

	try
	{
		std::string eventString= szUtf8EventString;

		if (eventString == WEBSOCKET_CONNECT_EVENT)
		{
			eventPtr= std::make_shared<MikanConnectedEvent>();
		}
		else if (eventString == WEBSOCKET_DISCONNECT_EVENT)
		{
			eventPtr= std::make_shared<MikanDisconnectedEvent>();
		}
		else if (eventString == WEBSOCKET_ERROR_EVENT)
		{
			MIKAN_MT_LOG_ERROR("MikanClient::parseEventString()") << "Received websocket " << eventString;
		}
		else if (eventString == WEBSOCKET_PING_EVENT)
		{
			MIKAN_MT_LOG_INFO("MikanClient::parseEventString()") << "Received websocket PING";
		}
		else if (eventString == WEBSOCKET_PONG_EVENT)
		{
			MIKAN_MT_LOG_INFO("MikanClient::parseEventString()") << "Received websocket PONG";
		}
		else
		{
			json jsonResponse = json::parse(eventString);
			std::string eventType = jsonResponse["eventType"].get<std::string>();

			rfk::Struct const* eventStruct = rfk::getDatabase().getFileLevelStructByName(eventType.c_str());
			if (eventStruct != nullptr)
			{
				eventPtr = eventStruct->makeSharedInstance<MikanEvent>();

				Serialization::deserializeFromJson(jsonResponse, eventPtr.get(), *eventStruct);
			}
			else
			{
				MIKAN_MT_LOG_WARNING("MikanClient::parseEventString()")
					<< "Received response for unknown eventType: " << eventType;
			}
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