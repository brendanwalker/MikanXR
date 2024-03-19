#include "BoostInterprocessMessageClient.h"
#include "Logger.h"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <chrono>

//-- BoostInterprocessMessageClient -----
BoostInterprocessMessageClient::BoostInterprocessMessageClient()
	: m_severEventQueue(nullptr)
	, m_functionCallQueue(nullptr)
	, m_functionResponseQueue(nullptr)
	, m_isConnected(false)
{
	memset(&m_clientInfo, 0, sizeof(MikanClientInfo));
}

BoostInterprocessMessageClient::~BoostInterprocessMessageClient()
{
	disconnect();
}

MikanResult BoostInterprocessMessageClient::connect(const std::string& clientId, MikanClientInfo* clientInfo)
{
	MikanResult resultCode = MikanResult_GeneralError;

	try
	{
		m_clientId = clientId;
		m_clientInfo = *clientInfo;
		m_serverEventQueueName = std::string(SERVER_EVENT_QUEUE_PREFIX) + m_clientId;
		m_functionResponseQueueName = std::string(FUNCTION_RESPONSE_QUEUE_PREFIX) + m_clientId;

		// Connect to the queue used to send remote function calls to the server
		m_functionCallQueue =
			new boost::interprocess::message_queue(
				boost::interprocess::open_only,
				FUNCTION_CALL_QUEUE_NAME);

		// Clear an stale client receive queues
		boost::interprocess::message_queue::remove(m_serverEventQueueName.c_str());
		boost::interprocess::message_queue::remove(m_functionResponseQueueName.c_str());

		// (re)create client function response and event receive queues
		m_functionResponseQueue =
			new boost::interprocess::message_queue(
				boost::interprocess::create_only,
				m_functionResponseQueueName.c_str(),
				16,
				sizeof(MikanRemoteFunctionResult));

		m_severEventQueue =
			new boost::interprocess::message_queue(
				boost::interprocess::create_only,
				m_serverEventQueueName.c_str(),
				128,
				sizeof(MikanEvent));

		// TODO: Send protocol version
		MikanRemoteFunctionCall connectionRequest(m_clientId.c_str(), CONNECT_FUNCTION_NAME, (uint8_t*)clientInfo, sizeof(MikanClientInfo));
		MikanRemoteFunctionResult connectionResult;
		resultCode = callRemoteFunction(&connectionRequest, &connectionResult);
		if (resultCode == MikanResult_Success)
		{
			resultCode = connectionResult.getResultCode();

			if (resultCode == MikanResult_Success || resultCode == MikanResult_AlreadyConnected)
			{
				if (resultCode == MikanResult_AlreadyConnected)
				{
					MIKAN_LOG_WARNING("InterprocessMessageClient::connect()") << "Already connected";
				}
				else
				{
					MIKAN_LOG_INFO("InterprocessMessageClient::connect()") << "Successfully connected";
				}

				m_isConnected = true;
				resultCode = MikanResult_Success;
			}
			else
			{
				MIKAN_LOG_ERROR("InterprocessMessageClient::connect()") << "Error connecting: " << resultCode;
				disconnect();
			}
		}
		else
		{
			MIKAN_LOG_ERROR("InterprocessMessageClient::connect()") << "Error connecting. Failed to send RPC: " << resultCode;
			disconnect();
		}
	}
	catch (boost::interprocess::interprocess_exception& ex)
	{
		disconnect();
		resultCode = MikanResult_SharedMemoryError;
	}

	return resultCode;
}

void BoostInterprocessMessageClient::disconnect()
{
	if (m_isConnected)
	{
		MikanRemoteFunctionCall disconnectRequest(m_clientId.c_str(), DISCONNECT_FUNCTION_NAME);
		MikanRemoteFunctionResult disconnectResult;
		MikanResult resultCode = callRemoteFunction(&disconnectRequest, &disconnectResult);

		if (resultCode == MikanResult_Success)
		{
			resultCode = disconnectResult.getResultCode();

			if (resultCode == MikanResult_Success)
			{
				MIKAN_LOG_WARNING("InterprocessMessageClient::connect()") << "Successfully disconnected";
			}
			else
			{
				MIKAN_LOG_WARNING("InterprocessMessageClient::disconnect()") << "Error disconnecting: " << resultCode;
			}
		}
		else
		{
			MIKAN_LOG_WARNING("InterprocessMessageClient::disconnect()") << "Error disconnecting. Failed to call RPC: " << resultCode;
		}

		m_isConnected = false;
	}



	if (m_functionCallQueue != nullptr)
	{
		delete m_functionCallQueue;
		m_functionCallQueue = nullptr;
	}

	if (m_severEventQueue != nullptr)
	{
		delete m_severEventQueue;
		m_severEventQueue = nullptr;

		if (!boost::interprocess::message_queue::remove(m_serverEventQueueName.c_str()))
		{
			MIKAN_LOG_WARNING("InterprocessMessageClient::disconnect()") << "Failed to delete server event queue file: " << m_serverEventQueueName;
		}
	}

	if (m_functionResponseQueue != nullptr)
	{
		delete m_functionResponseQueue;
		m_functionResponseQueue = nullptr;

		if (!boost::interprocess::message_queue::remove(m_functionResponseQueueName.c_str()))
		{
			MIKAN_LOG_WARNING("InterprocessMessageClient::disconnect()") << "Failed to delete function response queue file: " << m_functionResponseQueueName;
		}
	}
}

bool BoostInterprocessMessageClient::tryFetchNextServerEvent(MikanEvent* outEvent)
{
	if (m_severEventQueue == nullptr)
		return false;

	auto recvd_size = 0ULL;
	unsigned int priority = 0;
	bool bSuccess = m_severEventQueue->try_receive(outEvent, sizeof(MikanEvent), recvd_size, priority) && recvd_size > 0;

	// Special case for MikanXR disconnecting on the client
	if (bSuccess && outEvent && outEvent->event_type == MikanEvent_disconnected)
	{
		m_isConnected = false;
	}

	return bSuccess;
}

MikanResult BoostInterprocessMessageClient::callRemoteFunction(
	const MikanRemoteFunctionCall* inFunctionCall,
	MikanRemoteFunctionResult* outResult)
{
	if (m_functionCallQueue != nullptr)
	{
		if (m_functionCallQueue->try_send((void*)inFunctionCall, inFunctionCall->getTotalSize(), 0))
		{
			// Timeout after 10 milliseconds as a safety in case server crashes
			const std::chrono::time_point<std::chrono::system_clock> timeout =
				std::chrono::system_clock::now()
				+ std::chrono::seconds(1);

			auto recvd_size = 0ULL;
			unsigned int priority = 0;
			MikanRemoteFunctionResult result;
			bool bGotExpectedResponse = false;
			while (m_functionResponseQueue->timed_receive(
				(void*)&result, sizeof(MikanRemoteFunctionResult),
				recvd_size, priority,
				timeout))
			{
				if (inFunctionCall->getRequestId() == result.getRequestId())
				{
					*outResult = result;
					bGotExpectedResponse = true;
					break;
				}
			}

			return bGotExpectedResponse ? MikanResult_Success : MikanResult_FunctionResponseTimeout;
		}
		else
		{
			return MikanResult_FailedFunctionSend;
		}
	}
	else
	{
		return MikanResult_NotConnected;
	}
}

MikanResult BoostInterprocessMessageClient::callRemoteFunction(const char* functionName, MikanRemoteFunctionResult* outResult)
{
	MikanRemoteFunctionCall functionCall(m_clientId.c_str(), functionName);

	return callRemoteFunction(&functionCall, outResult);
}

MikanResult BoostInterprocessMessageClient::callRemoteFunction(
	const char* functionName,
	uint8_t* buffer,
	size_t bufferSize,
	MikanRemoteFunctionResult* outResult)
{
	MikanRemoteFunctionCall functionCall(m_clientId.c_str(), functionName, buffer, bufferSize);

	return callRemoteFunction(&functionCall, outResult);
}
