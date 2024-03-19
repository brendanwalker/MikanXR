#include "WebsocketInterprocessMessageServer.h"
#include "Logger.h"
#include "ThreadUtils.h"

#include "IxWebSocket/IXConnectionState.h"
#include "IxWebSocket/IxNetSystem.h"
#include "IxWebSocket/IXWebSocket.h"
#include "IxWebSocket/IXWebSocketServer.h"
#include "IxWebSocket/IXWebSocketSendData.h"

#include "readerwriterqueue.h"

#include <chrono>

using LockFreeRPCQueue = moodycamel::ReaderWriterQueue<MikanRemoteFunctionCall>;
using LockFreeRPCQueuePtr = std::shared_ptr<LockFreeRPCQueue>;
using WebSocketWeakPtr = std::weak_ptr<ix::WebSocket>;
using WebSocketPtr = std::shared_ptr<ix::WebSocket>;

#define WEBSOCKET_SERVER_PORT 8080

//-- ClientConnectionState -----
class ClientConnectionState : public ix::ConnectionState
{
public:
	ClientConnectionState() 		
		: ix::ConnectionState()
		, m_functionCallQueue(std::make_shared<LockFreeRPCQueue>())
	{}

	void bindWebSocket(WebSocketWeakPtr websocket)
	{
		m_websocket = websocket;
	}

	bool bindClientId(const std::string& clientId)
	{
		if (m_clientId.empty())
		{
			m_clientId = clientId;
			return true;
		}

		return false;	
	}

	bool disconnect()
	{
		WebSocketPtr websocket = m_websocket.lock();
		if (websocket)
		{
			websocket->close();
			return true;
		}

		return false;
	}

	const std::string getClientId() const { return m_clientId; }
	inline LockFreeRPCQueuePtr getFunctionCallQueue() { return m_functionCallQueue; }

	void handleClientMessage(
		ConnectionStatePtr connectionState,
		const ix::WebSocketMessagePtr& msg)
	{
		switch (msg->type)
		{
			case ix::WebSocketMessageType::Open:
				{
					auto remoteIp = connectionState->getRemoteIp();

					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
						<< "New connection";
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
						<< "remote ip: " << remoteIp;
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
						<< "id: " << connectionState->getId();
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
						<< "Uri: " << msg->openInfo.uri;

					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") << "Headers:";
					for (auto it : msg->openInfo.headers)
					{
						MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
							<< it.first << ": " << it.second;
					}
				}
				break;
			case ix::WebSocketMessageType::Close:
				{
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
						<< "Close connection";
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
						<< "id: " << connectionState->getId();
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
						<< "reason: " << msg->closeInfo.reason;
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
						<< "code: " << msg->closeInfo.code;
				}
				break;
			case ix::WebSocketMessageType::Message:
				{
					if (msg->binary)
					{
						// Read the message as a binary RPC call
						MikanRemoteFunctionCall functionCall;
						if (msg->str.size() <= sizeof(MikanRemoteFunctionCall))
						{
							memcpy(&functionCall, msg->str.c_str(), msg->str.size());

							// Enqueue the function call
							m_functionCallQueue->enqueue(functionCall);
						}
						else
						{
							MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
								<< "Received RPC call that was too large ( " << msg->str.size() << " bytes)";
						}
					}
				} break;
			case ix::WebSocketMessageType::Error:
				{
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") 
						<< "Error: " << msg->errorInfo.reason;
				}
				break;
			case ix::WebSocketMessageType::Ping:
				{
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") << "Ping";
				}
				break;
			case ix::WebSocketMessageType::Pong:
				{
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") << "Pong";
				}
				break;
			case ix::WebSocketMessageType::Fragment:
				{
					MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage") << "Fragment";
				}
				break;
		}
	}

	bool sendEvent(MikanEvent* event)
	{
		WebSocketPtr websocket = m_websocket.lock();

		if (websocket != nullptr)
		{
			auto sendInfo= 
				websocket->sendBinary(
					ix::IXWebSocketSendData((char*)event, sizeof(MikanEvent)));

			return sendInfo.success;
		}

		return false;
	}

	bool sendFunctionResponse(MikanRemoteFunctionResult* result)
	{
		WebSocketPtr websocket = m_websocket.lock();

		if (websocket)
		{
			auto sendInfo = 
				websocket->sendBinary(
					ix::IXWebSocketSendData((char*)result, result->getTotalSize()));

			return sendInfo.success;
		}

		return false;
	}

private:
	LockFreeRPCQueuePtr m_functionCallQueue;
	WebSocketWeakPtr m_websocket;
	std::string m_clientId;
};

//-- WebsocketInterprocessMessageServer -----
WebsocketInterprocessMessageServer::WebsocketInterprocessMessageServer()
	: m_server(nullptr)
{}

WebsocketInterprocessMessageServer::~WebsocketInterprocessMessageServer()
{
	dispose();
}

bool WebsocketInterprocessMessageServer::initialize()
{
	bool bSuccess = true;

	if (!ix::initNetSystem())
	{
		MIKAN_LOG_WARNING("WebsocketInterprocessMessageServer::initialize()") 
			<< "Failed to initialize net system";
		bSuccess = false;
	}

	if (bSuccess)
	{
		m_server = std::make_shared<ix::WebSocketServer>(WEBSOCKET_SERVER_PORT);

		auto connectionStateFactory = []() -> ClientConnectionStatePtr {
			return std::make_shared<ClientConnectionState>();
		};

		auto clientConnectCallback = [this](
			WebSocketWeakPtr webSocket, 
			ConnectionStatePtr connectionState) 
		{
			ClientConnectionStatePtr clientConnectionState = 
				std::static_pointer_cast<ClientConnectionState>(connectionState);

			// Bind the websocket to the connection state
			clientConnectionState->bindWebSocket(webSocket);

			// Add the connection to the list of connections
			// TODO: Is this thread safe?
			m_connections.push_back(clientConnectionState);
		};

		auto clientMessageCallback = [this](
			ConnectionStatePtr connectionState,
			ix::WebSocket& webSocket,
			const ix::WebSocketMessagePtr& msg) 
		{
			ClientConnectionStatePtr clientConnectionState =
				std::static_pointer_cast<ClientConnectionState>(connectionState);

			clientConnectionState->handleClientMessage(connectionState, msg);
		};

		m_server->setConnectionStateFactory(connectionStateFactory);
		m_server->setOnConnectionCallback(clientConnectCallback);
		m_server->setOnClientMessageCallback(clientMessageCallback);
	}

	return bSuccess;
}

void WebsocketInterprocessMessageServer::dispose()
{
	if (m_server)
	{
		// Disconnect all clients
		for (ClientConnectionStateWeakPtr weakPtr : m_connections)
		{
			ClientConnectionStatePtr connection = weakPtr.lock();
			if (connection)
			{
				// Tell the client that they are getting disconnected
				MikanEvent disconnectEvent;
				memset(&disconnectEvent, 0, sizeof(MikanEvent));
				disconnectEvent.event_type = MikanEvent_disconnected;
				connection->sendEvent(&disconnectEvent);

				// Close the connection
				connection->disconnect();
			}
		}
		m_connections.clear();

		// Give us 500ms for the server to notice that clients went away
		ThreadUtils::sleepMilliseconds(500);
		m_server->stop();
	}

	ix::uninitNetSystem();
}

void WebsocketInterprocessMessageServer::setRPCHandler(const std::string& functionName, RPCHandler handler)
{
	m_functionHandlers[functionName] = handler;
}

void WebsocketInterprocessMessageServer::sendServerEventToClient(const std::string& clientId, MikanEvent* event)
{
	for (ClientConnectionStateWeakPtr weakPtr : m_connections)
	{
		ClientConnectionStatePtr connection = weakPtr.lock();
		if (connection && connection->getClientId() == clientId)
		{
			connection->sendEvent(event);
		}
	}
}

void WebsocketInterprocessMessageServer::sendServerEventToAllClients(MikanEvent* event)
{
	for (ClientConnectionStateWeakPtr weakPtr : m_connections)
	{
		ClientConnectionStatePtr connection = weakPtr.lock();
		if (connection)
		{
			connection->sendEvent(event);
		}
	}
}

void WebsocketInterprocessMessageServer::processRemoteFunctionCalls()
{
	// Process all connections
	for (auto connection_it = m_connections.begin(); connection_it != m_connections.end(); connection_it++)	
	{
		ClientConnectionStatePtr connection = connection_it->lock();

		// Remove any dead connections
		if (!connection)
		{
			connection_it= m_connections.erase(connection_it);
			continue;
		}

		// Read all pending RPC in the queue
		MikanRemoteFunctionCall inFunctionCall;
		while (connection->getFunctionCallQueue()->try_dequeue(inFunctionCall))
		{
			const std::string clientId = inFunctionCall.getClientId();
			
			// Handle connect request
			if (strncmp(inFunctionCall.getFunctionName(), CONNECT_FUNCTION_NAME, strlen(CONNECT_FUNCTION_NAME)) == 0)
			{
				// Make sure the client isn't already connected
				if (connection->bindClientId(clientId))
				{
					bool bSuccess = false;

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
						MIKAN_LOG_INFO("processRemoteFunctionCalls") 
							<< "Connecting client: " << connection->getClientId();

						// Tell the client that they are now connected
						MikanEvent connectEvent;
						memset(&connectEvent, 0, sizeof(MikanEvent));
						connectEvent.event_type = MikanEvent_connected;
						connection->sendEvent(&connectEvent);

						bSuccess = true;
					}

					if (!bSuccess)
					{
						MIKAN_LOG_ERROR("processRemoteFunctionCalls") 
							<< "Failed to initialize connection for client: " << clientId;
						connection->disconnect();
					}
				}
				// Tell the client they are already connected
				else
				{
					MikanRemoteFunctionResult connectResponse(MikanResult_AlreadyConnected, inFunctionCall.getRequestId());

					if (!connection->sendFunctionResponse(&connectResponse))
					{
						MIKAN_LOG_WARNING("processRemoteFunctionCalls") << "Failed to tell client they are already connected: " << clientId;
					}
				}
			}
			// Handle disconnect request
			else if (strncmp(inFunctionCall.getFunctionName(), DISCONNECT_FUNCTION_NAME, strlen(DISCONNECT_FUNCTION_NAME)) == 0)
			{
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
				connection->disconnect();
			}
			// Handle all other requests
			else
			{
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
		}
	}
}