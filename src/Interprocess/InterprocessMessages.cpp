#include "InterprocessMessages.h"
#include "Logger.h"
#include <boost/interprocess/ipc/message_queue.hpp>
#include <chrono>

//-- MikanRemoteFunctionCall -----
uint32_t MikanRemoteFunctionCall::m_nextRequestId= 0;

MikanRemoteFunctionCall::MikanRemoteFunctionCall()
{
	m_header.clientId[0]= '\0';
	m_header.functionName[0]= '\0';
	m_header.requestId= m_nextRequestId++;
	m_header.parameterBufferSize= 0;
}

MikanRemoteFunctionCall::MikanRemoteFunctionCall(
	const char* clientId, 
	const char* functionName)
{
	m_header.requestId = m_nextRequestId++;
	setClientId(clientId);
	setFunctionName(functionName);
	setParameterBuffer(nullptr, 0);
}

MikanRemoteFunctionCall::MikanRemoteFunctionCall(
	const char* clientId, 
	const char* functionName, 
	uint8_t* buffer, 
	size_t bufferSize)
{
	m_header.requestId = m_nextRequestId++;
	setClientId(clientId);
	setFunctionName(functionName);
	setParameterBuffer(buffer, bufferSize);
}

void MikanRemoteFunctionCall::setClientId(const char* clientId)
{
	const size_t clientIdLength= strlen(clientId);
	assert(clientIdLength < sizeof(m_header.clientId));
	memset(m_header.clientId, 0, sizeof(m_header.clientId));
	strncpy(m_header.clientId, clientId, sizeof(m_header.clientId) - 1);
}

void MikanRemoteFunctionCall::setFunctionName(const char* functionName)
{
	const size_t functionNameLength = strlen(functionName);
	assert(functionNameLength < sizeof(m_header.functionName));
	memset(m_header.functionName, 0, sizeof(m_header.functionName));
	strncpy(m_header.functionName, functionName, sizeof(m_header.functionName) - 1);
}

void MikanRemoteFunctionCall::setParameterBuffer(uint8_t* buffer, size_t bufferSize)
{
	assert(bufferSize <= sizeof(m_parameterBuffer));

	if (buffer != nullptr && bufferSize > 0 && bufferSize <= sizeof(m_parameterBuffer))
	{
		std::memcpy(m_parameterBuffer, buffer, bufferSize);
		m_header.parameterBufferSize= bufferSize;
	}
	else
	{
		m_header.parameterBufferSize = 0;
	}
}

size_t MikanRemoteFunctionCall::getTotalSize() const
{
	return sizeof(m_header) + m_header.parameterBufferSize;
}

//-- MikanRemoteFunctionResult ---- 
MikanRemoteFunctionResult::MikanRemoteFunctionResult()
{
	m_header.resultCode= MikanResult_Success;
	m_header.requestId= 0;
	m_header.resultBufferSize = 0;
}

MikanRemoteFunctionResult::MikanRemoteFunctionResult(
	MikanResult result, 
	uint32_t requestId)
{
	setResultCode(result);
	setRequestId(requestId);
	setResultBuffer(nullptr, 0);
}

MikanRemoteFunctionResult::MikanRemoteFunctionResult(
	MikanResult result,
	uint32_t requestId,
	uint8_t* buffer,
	size_t bufferSize)
{
	setResultCode(result);
	setRequestId(requestId);
	setResultBuffer(buffer, bufferSize);
}

void MikanRemoteFunctionResult::setResultCode(MikanResult result)
{
	m_header.resultCode= result;
}

void MikanRemoteFunctionResult::setRequestId(uint32_t requestId)
{
	m_header.requestId= requestId;
}

void MikanRemoteFunctionResult::setResultBuffer(uint8_t* buffer, size_t bufferSize)
{
	assert(bufferSize <= sizeof(m_resultBuffer));

	if (buffer != nullptr && bufferSize > 0 && bufferSize <= sizeof(m_resultBuffer))
	{
		std::memcpy(m_resultBuffer, buffer, bufferSize);
		m_header.resultBufferSize = bufferSize;
	}
	else
	{
		m_header.resultBufferSize = 0;
	}
}

size_t MikanRemoteFunctionResult::getTotalSize() const
{
	return sizeof(m_header) + m_header.resultBufferSize;
}

//-- InterprocessMessageClient -----
InterprocessMessageClient::InterprocessMessageClient()
	: m_severEventQueue(nullptr)
	, m_functionCallQueue(nullptr)
	, m_functionResponseQueue(nullptr)
	, m_isConnected(false)
{
	memset(&m_clientInfo, 0, sizeof(MikanClientInfo));
}

InterprocessMessageClient::~InterprocessMessageClient()
{
	disconnect();
}

MikanResult InterprocessMessageClient::connect(const std::string& clientId, MikanClientInfo* clientInfo)
{
	MikanResult resultCode = MikanResult_GeneralError;

	try
	{
		m_clientId= clientId;
		m_clientInfo= *clientInfo;
		m_serverEventQueueName= std::string(SERVER_EVENT_QUEUE_PREFIX) + m_clientId;
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
		MikanRemoteFunctionCall connectionRequest(m_clientId.c_str(), CONNECT_FUNCTION_NAME, (uint8_t *)clientInfo, sizeof(MikanClientInfo));
		MikanRemoteFunctionResult connectionResult;
		resultCode = callRemoteFunction(&connectionRequest, &connectionResult);
		if (resultCode == MikanResult_Success)
		{
			resultCode= connectionResult.getResultCode();

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
				resultCode= MikanResult_Success;
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

void InterprocessMessageClient::disconnect()
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

		m_isConnected= false;
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

bool InterprocessMessageClient::tryFetchNextServerEvent(MikanEvent* outEvent)
{
	if (m_severEventQueue == nullptr)
		return false;

	auto recvd_size= 0ULL;
	unsigned int priority= 0;
	bool bSuccess= m_severEventQueue->try_receive(outEvent, sizeof(MikanEvent), recvd_size, priority) && recvd_size > 0;

	// Special case for MikanXR disconnecting on the client
	if (bSuccess && outEvent && outEvent->event_type == MikanEvent_disconnected)
	{
		m_isConnected= false;
	}

	return bSuccess;
}

MikanResult InterprocessMessageClient::callRemoteFunction(
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

MikanResult InterprocessMessageClient::callRemoteFunction(const char* functionName, MikanRemoteFunctionResult* outResult)
{
	MikanRemoteFunctionCall functionCall(m_clientId.c_str(), functionName);

	return callRemoteFunction(&functionCall, outResult);
}

MikanResult InterprocessMessageClient::callRemoteFunction(
	const char* functionName, 
	uint8_t* buffer, 
	size_t bufferSize, 
	MikanRemoteFunctionResult* outResult)
{
	MikanRemoteFunctionCall functionCall(m_clientId.c_str(), functionName, buffer, bufferSize);

	return callRemoteFunction(&functionCall, outResult);
}

//-- InterprocessMessageConnection -----
InterprocessMessageConnection::InterprocessMessageConnection()
	: m_eventQueue(nullptr)
	, m_functionResponseQueue(nullptr)
{
}

InterprocessMessageConnection::~InterprocessMessageConnection()
{
	dispose();
}

bool InterprocessMessageConnection::initialize(const std::string& clientId)
{
	bool bSuccess = false;

	try
	{
		// TODO: verify protocol version
		m_clientId= clientId;
		m_serverEventQueueName = std::string(SERVER_EVENT_QUEUE_PREFIX) + clientId;
		m_functionResponseQueueName = std::string(FUNCTION_RESPONSE_QUEUE_PREFIX) + clientId;

		// Connect to the event and response queues the client created
		m_functionResponseQueue =
			new boost::interprocess::message_queue(
				boost::interprocess::open_only,
				m_functionResponseQueueName.c_str());
		m_eventQueue =
			new boost::interprocess::message_queue(
				boost::interprocess::open_only,
				m_serverEventQueueName.c_str());

		bSuccess= true;
	}
	catch (boost::interprocess::interprocess_exception& ex)
	{
		dispose();
		bSuccess = false;
	}

	return bSuccess;
}

void InterprocessMessageConnection::dispose()
{
	if (m_eventQueue != nullptr)
	{
		delete m_eventQueue;
		m_eventQueue = nullptr;
	}

	if (m_functionResponseQueue != nullptr)
	{
		delete m_functionResponseQueue;
		m_functionResponseQueue = nullptr;
	}
}

bool InterprocessMessageConnection::sendEvent(MikanEvent* event)
{
	return m_eventQueue->try_send((void*)event, sizeof(MikanEvent), 0);
}

bool InterprocessMessageConnection::sendFunctionResponse(MikanRemoteFunctionResult* result)
{
	return m_functionResponseQueue->try_send((void*)result, result->getTotalSize(), 0);
}

//-- InterprocessMessageServer -----
InterprocessMessageServer::InterprocessMessageServer()
	: m_functionCallQueue(nullptr)
{
}

InterprocessMessageServer::~InterprocessMessageServer()
{
	dispose();
}

bool InterprocessMessageServer::initialize()
{
	try
	{
		// Clean up any stale function call queue
		boost::interprocess::message_queue::remove(FUNCTION_CALL_QUEUE_NAME);

		// Create the function call queue for incoming requests
		m_functionCallQueue =
			new boost::interprocess::message_queue(
				boost::interprocess::create_only,
				FUNCTION_CALL_QUEUE_NAME,
				128,
				sizeof(MikanRemoteFunctionCall));
	}
	catch (boost::interprocess::interprocess_exception& ex)
	{
		dispose();
		return false;
	}

	return true;
}

void InterprocessMessageServer::dispose()
{
	for (const auto& kv : m_connections)
	{
		// Tell the client that they are getting disconnected
		MikanEvent disconnectEvent;
		memset(&disconnectEvent, 0, sizeof(MikanEvent));
		disconnectEvent.event_type = MikanEvent_disconnected;
		kv.second->sendEvent(&disconnectEvent);

		// Clean up the connection
		kv.second->dispose();
		delete kv.second;
	}
	m_connections.clear();

	if (m_functionCallQueue != nullptr)
	{
		delete m_functionCallQueue;
		m_functionCallQueue = nullptr;

		if (!boost::interprocess::message_queue::remove(FUNCTION_CALL_QUEUE_NAME))
		{
			MIKAN_LOG_WARNING("InterprocessMessageClient::disconnect()") << "Failed to delete function call queue file: " << FUNCTION_CALL_QUEUE_NAME;
		}
	}
}

void InterprocessMessageServer::setRPCHandler(const std::string& functionName, RPCHandler handler)
{
	m_functionHandlers[functionName] = handler;
}

void InterprocessMessageServer::sendServerEventToClient(const std::string& clientId, MikanEvent* event)
{
	auto connection_it = m_connections.find(clientId);
	if (connection_it != m_connections.end())
	{
		connection_it->second->sendEvent(event);
	}
}

void InterprocessMessageServer::sendServerEventToAllClients(MikanEvent* event)
{
	for (const auto& kv : m_connections)
	{
		kv.second->sendEvent(event);
	}
}

void InterprocessMessageServer::processRemoteFunctionCalls()
{
	MikanRemoteFunctionCall inFunctionCall;
	auto recvd_size= 0ULL;
	unsigned int priority= 0;
	while (m_functionCallQueue->try_receive(&inFunctionCall, sizeof(MikanRemoteFunctionCall), recvd_size, priority) 
			&& recvd_size > 0)
	{
		const std::string clientId = inFunctionCall.getClientId();
		auto connection_it = m_connections.find(clientId);

		// Handle connect request
		if (strncmp(inFunctionCall.getFunctionName(), CONNECT_FUNCTION_NAME, strlen(CONNECT_FUNCTION_NAME)) == 0)
		{
			// Make sure the client isn't already connected
			if (connection_it == m_connections.end())
			{
				InterprocessMessageConnection* connection = new InterprocessMessageConnection();
				bool bSuccess= false;

				if (connection->initialize(clientId))
				{
					MikanRemoteFunctionResult connectResponse(MikanResult_Success, inFunctionCall.getRequestId());

					// Get the response from a registered function handler, if any
					auto handler_it = m_functionHandlers.find(CONNECT_FUNCTION_NAME);
					if (handler_it != m_functionHandlers.end())
					{
						handler_it->second(&inFunctionCall, &connectResponse);
					}

					// Send connection reply back to the client
					if (connection->sendFunctionResponse(&connectResponse))
					{
						MIKAN_LOG_INFO("processRemoteFunctionCalls") << "Connecting client: " << connection->getClientId();
						m_connections.insert({ clientId, connection });

						// Tell the client that they are now connected
						MikanEvent connectEvent;
						memset(&connectEvent, 0, sizeof(MikanEvent));
						connectEvent.event_type = MikanEvent_connected;
						connection->sendEvent(&connectEvent);

						bSuccess= true;
					}
				}

				if (!bSuccess)
				{
					MIKAN_LOG_ERROR("processRemoteFunctionCalls") << "Failed to initialize connection for client: " << clientId;
					connection->dispose();
					delete connection;
				}
			}
			// Tell the client they are already connected
			else
			{
				MikanRemoteFunctionResult connectResponse(MikanResult_AlreadyConnected, inFunctionCall.getRequestId());
				InterprocessMessageConnection* connection = connection_it->second;

				if (!connection->sendFunctionResponse(&connectResponse))
				{
					MIKAN_LOG_WARNING("processRemoteFunctionCalls") << "Failed to tell client they are already connected: " << clientId;
				}
			}
		}
		// Handle disconnect request
		else if (strncmp(inFunctionCall.getFunctionName(), DISCONNECT_FUNCTION_NAME, strlen(DISCONNECT_FUNCTION_NAME)) == 0)
		{
			if (connection_it != m_connections.end())
			{
				InterprocessMessageConnection* connection = connection_it->second;
				MikanRemoteFunctionResult connectResponse(MikanResult_Success, inFunctionCall.getRequestId());

				// Get the response from a registered function handler, if any
				auto handler_it = m_functionHandlers.find(DISCONNECT_FUNCTION_NAME);
				if (handler_it != m_functionHandlers.end())
				{
					handler_it->second(&inFunctionCall, &connectResponse);
				}

				// Acknowledge the disconnection request
				connection->sendFunctionResponse(&connectResponse);

				// Clean up the connection state
				connection->dispose();
				delete connection;

				// Remove the connection from the connection list
				m_connections.erase(connection_it);
			}
			else
			{
				MIKAN_LOG_WARNING("processRemoteFunctionCalls") << "Ignoring connection request from existing client: " << clientId;
			}
		}
		// Handle all other requests
		else
		{
			if (connection_it != m_connections.end())
			{
				InterprocessMessageConnection* connection = connection_it->second;
				const std::string functionName= inFunctionCall.getFunctionName();

				MikanRemoteFunctionResult outResult;
				outResult.setResultCode(MikanResult_Success);
				outResult.setRequestId(inFunctionCall.getRequestId());

				// Get the response from a registered function handler
				auto handler_it = m_functionHandlers.find(functionName);
				if (handler_it != m_functionHandlers.end())
				{
					handler_it->second(&inFunctionCall, &outResult);
				}
				else
				{
					outResult.setResultCode(MikanResult_UnknownFunction);
				}

				// Send the response back to the client
				connection->sendFunctionResponse(&outResult);
			}
			else
			{
				MIKAN_LOG_ERROR("processRemoteFunctionCalls") << "Received call from unknown client: " << clientId;
			}
		}
	}
}