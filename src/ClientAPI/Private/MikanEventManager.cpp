#include "MikanEventManager.h"
#include "MikanAPITypes.h"
#include "MikanCoreCAPI.h"
#include "MikanClientEvents.h"
#include "Logger.h"
#include "JsonDeserializer.h"
#include "StringUtils.h"

#include <Refureku/Refureku.h>
#include <string>

#include "nlohmann/json.hpp"

#define WEBSOCKET_CONNECT_EVENT				"connect"
#define WEBSOCKET_DISCONNECT_EVENT			"disconnect"
#define WEBSOCKET_ERROR_EVENT				"error"
#define WEBSOCKET_PING_EVENT				"ping"
#define WEBSOCKET_PONG_EVENT				"pong"

using json = nlohmann::json;

MikanAPIResult MikanEventManager::init(MikanContext context)
{
	m_context = context;

	return MikanAPIResult::Success;
}

MikanAPIResult MikanEventManager::fetchNextEvent(MikanEventPtr& out_event)
{
	char utf8Buffer[1024];
	size_t utf8BytesWritten = 0;

	MikanAPIResult result = 
		(MikanAPIResult)Mikan_FetchNextEvent(
			m_context, sizeof(utf8Buffer), utf8Buffer, &utf8BytesWritten);
	if (result == MikanAPIResult::Success)
	{
		out_event = parseEventString(utf8Buffer);
		if (!out_event)
		{
			MIKAN_MT_LOG_WARNING("MikanClient::fetchNextEvent()")
				<< "Failed to parse event string: " << utf8Buffer;
			result = MikanAPIResult::MalformedResponse;
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

		if (eventString.rfind(WEBSOCKET_CONNECT_EVENT, 0) == 0)
		{
			int clientVersion = Mikan_GetClientAPIVersion();
			int minClientVersion = 0;
			int serverVersion = 0;

			std::vector<std::string> tokens= StringUtils::splitString(eventString, ':');
			if (tokens.size() >= 3)
			{
				serverVersion = std::atoi(tokens[1].c_str());
				minClientVersion = std::atoi(tokens[2].c_str());
			}
			
			// Make sure the client version isn't too old
			if (clientVersion >= minClientVersion)
			{
				auto connectEventPtr = std::make_shared<MikanConnectedEvent>();
				connectEventPtr->serverVersion.version = serverVersion;
				connectEventPtr->minClientVersion.version = minClientVersion;

				eventPtr = connectEventPtr;
			}
			else
			{
				// Disconnect since we have incompatible client
				// This will trigger an WEBSOCKET_DISCONNECT_EVENT
				Mikan_Disconnect(
					m_context,
					(uint16_t)MikanDisconnectCode_IncompatibleVersion,
					"Incompatible client version");
			}
		}
		else if (eventString.rfind(WEBSOCKET_DISCONNECT_EVENT, 0) == 0)
		{
			int disconnectCode = 0;
			std::string disconnectReason = "";

			std::vector<std::string> tokens = StringUtils::splitString(eventString, ':');
			if (tokens.size() >= 3)
			{
				disconnectCode = std::atoi(tokens[1].c_str());
				disconnectReason = tokens[2].c_str();
			}

			auto disconnectEventPtr= std::make_shared<MikanDisconnectedEvent>();
			disconnectEventPtr->code = (MikanDisconnectCode)disconnectCode;
			disconnectEventPtr->reason.setValue(disconnectReason);

			eventPtr= disconnectEventPtr;
		}
		else if (eventString == WEBSOCKET_ERROR_EVENT)
		{
			MIKAN_MT_LOG_ERROR("MikanClient::parseEventString()") << "Received websocket ERROR " << eventString;
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
			uint64_t eventTypeId = jsonResponse["eventTypeId"].get<uint64_t>();

			rfk::Struct const* eventStruct = rfk::getDatabase().getStructById(eventTypeId);
			if (eventStruct != nullptr)
			{
				eventPtr = eventStruct->makeSharedInstance<MikanEvent>();

				Serialization::deserializeFromJson(jsonResponse, eventPtr.get(), *eventStruct);
			}
			else
			{
				MIKAN_MT_LOG_WARNING("MikanClient::parseEventString()")
					<< "Received response for unknown eventTypeId: " << eventTypeId;
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