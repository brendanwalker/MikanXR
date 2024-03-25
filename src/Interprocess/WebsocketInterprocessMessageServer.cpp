#include "WebsocketInterprocessMessageServer.h"
#include "MikanJsonUtils.h"
#include "MikanEventTypes.h"
#include "MikanEventTypes_json.h"
#include "MikanClientTypes.h"
#include "MikanClientTypes_json.h"
#include "Logger.h"
#include "StringUtils.h"
#include "ThreadUtils.h"

#include "IxWebSocket/IXConnectionState.h"
#include "IxWebSocket/IxNetSystem.h"
#include "IxWebSocket/IXWebSocket.h"
#include "IxWebSocket/IXWebSocketServer.h"
#include "IxWebSocket/IXWebSocketSendData.h"

#include "nlohmann/json.hpp"

#include "readerwriterqueue.h"

#include <chrono>

using json = nlohmann::json;

using LockFreeRequestQueue = moodycamel::ReaderWriterQueue<std::string>;
using LockFreeRequestQueuePtr = std::shared_ptr<LockFreeRequestQueue>;
using WebSocketWeakPtr = std::weak_ptr<ix::WebSocket>;
using WebSocketPtr = std::shared_ptr<ix::WebSocket>;

//-- ClientConnectionState -----
class ClientConnectionState : public ix::ConnectionState
{
public:
	ClientConnectionState() 		
		: ix::ConnectionState()
		, m_functionCallQueue(std::make_shared<LockFreeRequestQueue>())
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
	inline LockFreeRequestQueuePtr getFunctionCallQueue() { return m_functionCallQueue; }

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
					if (!msg->binary)
					{
						// Enqueue the request json string
						m_functionCallQueue->enqueue(msg->str);
					}
					else
					{
						MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage")
							<< "Received unsupported binary message";
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

	bool sendMessage(const std::string& response)
	{
		WebSocketPtr websocket = m_websocket.lock();

		if (websocket)
		{
			auto sendInfo = websocket->sendText(response);

			return sendInfo.success;
		}

		return false;
	}

private:
	LockFreeRequestQueuePtr m_functionCallQueue;
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
		m_server = std::make_shared<ix::WebSocketServer>();

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
				MikanDisconnectedEvent disconnectEvent;
				disconnectEvent.eventType = MikanDisconnectedEvent::k_typeName;

				json eventJson = disconnectEvent;
				std::string eventJsonString = eventJson.dump();

				connection->sendMessage(eventJsonString);

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

std::string WebsocketInterprocessMessageServer::makeRequestHandlerKey(const std::string& requestType, int version)
{
	return StringUtils::stringify(requestType, version);
}

void WebsocketInterprocessMessageServer::setRequestHandler(
	const std::string& functionName, 
	RequestHandler handler,
	int version)
{
	const std::string key = makeRequestHandlerKey(functionName, version);

	m_requestHandlers[key] = handler;
}

void WebsocketInterprocessMessageServer::sendMessageToClient(const std::string& clientId, const std::string& message)
{
	for (ClientConnectionStateWeakPtr weakPtr : m_connections)
	{
		ClientConnectionStatePtr connection = weakPtr.lock();
		if (connection && connection->getClientId() == clientId)
		{
			connection->sendMessage(message);
		}
	}
}

void WebsocketInterprocessMessageServer::sendMessageToAllClients(const std::string& message)
{
	for (ClientConnectionStateWeakPtr weakPtr : m_connections)
	{
		ClientConnectionStatePtr connection = weakPtr.lock();
		if (connection)
		{
			connection->sendMessage(message);
		}
	}
}

void WebsocketInterprocessMessageServer::processRequests()
{
	// Process all connections
	for (auto connection_it = m_connections.begin(); connection_it != m_connections.end(); connection_it++)	
	{
		ClientConnectionStatePtr connection = connection_it->lock();
		const std::string clientId = connection->getClientId();

		// Remove any dead connections
		if (!connection)
		{
			connection_it= m_connections.erase(connection_it);
			continue;
		}

		// Read all pending RPC in the queue
		std::string inRequestString;
		while (connection->getFunctionCallQueue()->try_dequeue(inRequestString))
		{
			std::string requestType;
			JsonSaxStringValueSearcher typeNameSearcher;
			if (!typeNameSearcher.fetchKeyValuePair(inRequestString, "requestType", requestType) || 
				requestType.empty())
			{
				MIKAN_LOG_WARNING("processRequests") << 
					"Request missing/invalid requestType field: " << inRequestString;
				continue;
			}

			json::number_integer_t version;
			JsonSaxIntegerValueSearcher versionSearcher;
			if (!versionSearcher.fetchKeyValuePair(inRequestString, "version", version) || 
				version < 0)
			{
				MIKAN_LOG_WARNING("processRequests") << 
					"Request missing/invalid version field: " << inRequestString;
				continue;
			}

			// Request ID is optional if the request doesn't expect a response
			json::number_integer_t requestId;
			JsonSaxIntegerValueSearcher requestIdSearcher;
			if (requestIdSearcher.fetchKeyValuePair(inRequestString, "requestId", requestId))
			{
				requestId= INVALID_MIKAN_ID;
			}

			// Find the handler for the request
			const std::string handlerKey = makeRequestHandlerKey(requestType, version);

			// Get the response from a registered function handler, if any
			std::string outResponseString;
			auto handler_it = m_requestHandlers.find(handlerKey);
			if (handler_it != m_requestHandlers.end())
			{
				ClientRequest request= { clientId, requestId, inRequestString };

				handler_it->second(request, outResponseString);
			}
			else
			{
				MikanResponse outResult;
				outResult.responseType = MikanResponse::k_typeName;
				outResult.requestId= requestId;
				outResult.resultCode= MikanResult_UnknownFunction;

				json responseJson = outResult;
				outResponseString = responseJson.dump();
			}

			// Send the response back to the client
			connection->sendMessage(outResponseString);

			// Handle connect request
		#if 0
			if (strncmp(requestType, CONNECT_FUNCTION_NAME, strlen(CONNECT_FUNCTION_NAME)) == 0)
			{
				// Make sure the client isn't already connected
				if (connection->bindClientId(clientId))
				{
					bool bSuccess = false;

					MikanRemoteFunctionResult connectResponse(MikanResult_Success, inRequestString.getRequestId());

					// Get the response from a registered function handler, if any
					auto handler_it = m_requestHandlers.find(CONNECT_FUNCTION_NAME);
					if (handler_it != m_requestHandlers.end())
					{
						handler_it->second(&inRequestString, &connectResponse);
					}

					// Send connection reply back to the client
					if (connection->sendMessage(&connectResponse))
					{
						MIKAN_LOG_INFO("processRemoteFunctionCalls") 
							<< "Connecting client: " << connection->getClientId();

						// Tell the client that they are now connected
						MikanEvent connectEvent;
						memset(&connectEvent, 0, sizeof(MikanEvent));
						connectEvent.event_type = MikanEvent_connected;
						connection->sendMessage(&connectEvent);

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
					MikanRemoteFunctionResult connectResponse(MikanResult_AlreadyConnected, inRequestString.getRequestId());

					if (!connection->sendMessage(&connectResponse))
					{
						MIKAN_LOG_WARNING("processRemoteFunctionCalls") << "Failed to tell client they are already connected: " << clientId;
					}
				}
			}
			// Handle disconnect request
			else if (strncmp(inRequestString.getFunctionName(), DISCONNECT_FUNCTION_NAME, strlen(DISCONNECT_FUNCTION_NAME)) == 0)
			{
				MikanRemoteFunctionResult connectResponse(MikanResult_Success, inRequestString.getRequestId());

				// Get the response from a registered function handler, if any
				auto handler_it = m_requestHandlers.find(DISCONNECT_FUNCTION_NAME);
				if (handler_it != m_requestHandlers.end())
				{
					handler_it->second(&inRequestString, &connectResponse);
				}

				// Acknowledge the disconnection request
				connection->sendMessage(&connectResponse);

				// Clean up the connection state
				connection->disconnect();
			}
			// Handle all other requests
			else
			{
				const std::string functionName = inRequestString.getFunctionName();

				MikanRemoteFunctionResult outResult;
				outResult.setResultCode(MikanResult_Success);
				outResult.setRequestId(inRequestString.getRequestId());

				// Get the response from a registered function handler
				auto handler_it = m_requestHandlers.find(functionName);
				if (handler_it != m_requestHandlers.end())
				{
					handler_it->second(&inRequestString, &outResult);
				}
				else
				{
					outResult.setResultCode(MikanResult_UnknownFunction);
				}

				// Send the response back to the client
				connection->sendMessage(&outResult);
			}
		#endif
		}
	}
}