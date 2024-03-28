#include "MikanEventManager.h"
#include "MikanAPITypes.h"
#include "MikanCoreCAPI.h"
#include "Logger.h"

#include "MikanEventTypes_json.h"

MikanResult MikanEventManager::fetchNextEvent(MikanEventPtr& out_event)
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

MikanEventPtr MikanEventManager::parseEventString(const char* utf8EventString)
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