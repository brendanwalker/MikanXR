#include "BoostInterprocessMessageServer.h"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <chrono>

#include "Logger.h"

//-- InterprocessMessageConnection -----
BoostInterprocessMessageConnection::BoostInterprocessMessageConnection()
	: m_eventQueue(nullptr)
	, m_functionResponseQueue(nullptr)
{}

BoostInterprocessMessageConnection::~BoostInterprocessMessageConnection()
{
	dispose();
}

bool BoostInterprocessMessageConnection::initialize(const std::string& clientId)
{
	bool bSuccess = false;

	try
	{
		// TODO: verify protocol version
		m_clientId = clientId;
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

		bSuccess = true;
	}
	catch (boost::interprocess::interprocess_exception& ex)
	{
		dispose();
		bSuccess = false;
	}

	return bSuccess;
}

void BoostInterprocessMessageConnection::dispose()
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

bool BoostInterprocessMessageConnection::sendEvent(MikanEvent* event)
{
	return m_eventQueue->try_send((void*)event, sizeof(MikanEvent), 0);
}

bool BoostInterprocessMessageConnection::sendFunctionResponse(MikanRemoteFunctionResult* result)
{
	return m_functionResponseQueue->try_send((void*)result, result->getTotalSize(), 0);
}

//-- BoostInterprocessMessageServer -----
BoostInterprocessMessageServer::BoostInterprocessMessageServer()
	: m_functionCallQueue(nullptr)
{}

BoostInterprocessMessageServer::~BoostInterprocessMessageServer()
{
	dispose();
}

bool BoostInterprocessMessageServer::initialize()
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

void BoostInterprocessMessageServer::dispose()
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

void BoostInterprocessMessageServer::setRPCHandler(const std::string& functionName, RPCHandler handler)
{
	m_functionHandlers[functionName] = handler;
}

void BoostInterprocessMessageServer::sendServerEventToClient(const std::string& clientId, MikanEvent* event)
{
	auto connection_it = m_connections.find(clientId);
	if (connection_it != m_connections.end())
	{
		connection_it->second->sendEvent(event);
	}
}

void BoostInterprocessMessageServer::sendServerEventToAllClients(MikanEvent* event)
{
	for (const auto& kv : m_connections)
	{
		kv.second->sendEvent(event);
	}
}

void BoostInterprocessMessageServer::processRemoteFunctionCalls()
{
	MikanRemoteFunctionCall inFunctionCall;
	auto recvd_size = 0ULL;
	unsigned int priority = 0;
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
				BoostInterprocessMessageConnection* connection = new BoostInterprocessMessageConnection();
				bool bSuccess = false;

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
						m_connections.insert({clientId, connection});

						// Tell the client that they are now connected
						MikanEvent connectEvent;
						memset(&connectEvent, 0, sizeof(MikanEvent));
						connectEvent.event_type = MikanEvent_connected;
						connection->sendEvent(&connectEvent);

						bSuccess = true;
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
				BoostInterprocessMessageConnection* connection = connection_it->second;

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
				BoostInterprocessMessageConnection* connection = connection_it->second;
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
				BoostInterprocessMessageConnection* connection = connection_it->second;
				const std::string functionName = inFunctionCall.getFunctionName();

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