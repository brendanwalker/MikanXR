#include "MikanEventManager.h"
#include "MikanAPITypes.h"
#include "MikanCoreCAPI.h"
#include "Logger.h"
#include "JsonDeserializer.h"

#include "MikanEventTypes_json.h"

#include <Refureku/Refureku.h>
#include <easy/profiler.h>

MikanResult MikanEventManager::fetchNextEvent(MikanEventPtr& out_event)
{
	char utf8Buffer[1024];
	size_t utf8BytesWritten = 0;

	MikanResult result = Mikan_FetchNextEvent(sizeof(utf8Buffer), utf8Buffer, &utf8BytesWritten);
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

MikanEventPtr MikanEventManager::parseEventString(const char* utf8EventString)
{
	EASY_FUNCTION();

	MikanEventPtr eventPtr;

	try
	{
		json jsonResponse = json::parse(utf8EventString);
		std::string eventType = jsonResponse["eventType"].get<std::string>();

		std::shared_ptr<MikanEvent> eventPtr;
		{
			EASY_BLOCK("Reflection Deserialization");

			rfk::Struct const* eventStruct = rfk::getDatabase().getFileLevelStructByName(eventType.c_str());
			if (eventStruct != nullptr)
			{
				eventPtr = eventStruct->makeSharedInstance<MikanEvent>();

				Serialization::deserializeFromJson(jsonResponse, eventPtr.get(), *eventStruct);
			}
		}

		auto it = m_eventFactories.find(eventType);
		if (it != m_eventFactories.end())
		{
			EASY_BLOCK("Original Deserialization");

			IMikanEventFactoryPtr factory = it->second;

			eventPtr = factory->createEvent(jsonResponse);
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