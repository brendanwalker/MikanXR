#include "WebsocketInterprocessMessageServer.h"
#include "JsonUtils.h"
#include "MikanClientEvents.h"
#include "MikanScriptEvents.h"
#include "MikanStencilEvents.h"
#include "MikanSpatialAnchorEvents.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVRDeviceEvents.h"
#include "Logger.h"
#include "StringUtils.h"
#include "ThreadUtils.h"
#include "JsonSerializer.h"
#include "JsonDeserializer.h"

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

//-- WebSocketClientConnection -----
class WebSocketClientConnection : public ix::ConnectionState
{
public:
	WebSocketClientConnection() 		
		: ix::ConnectionState()
		, m_functionCallQueue(std::make_shared<LockFreeRequestQueue>())
	{}

	void bindWebSocket(WebSocketWeakPtr websocket)
	{
		m_websocket = websocket;
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

	//const std::string getClientId() const { return m_clientInfo.clientId; }
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

					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") 
						<< "New connection";
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") 
						<< "remote ip: " << remoteIp;
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") 
						<< "id: " << connectionState->getId();
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") 
						<< "Uri: " << msg->openInfo.uri;
				}
				break;
			case ix::WebSocketMessageType::Close:
				{
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") 
						<< "Close connection";
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") 
						<< "id: " << connectionState->getId();
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") 
						<< "reason: " << msg->closeInfo.reason;
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") 
						<< "code: " << msg->closeInfo.code;

					// Serialize the client info to json
					json clientInfoPayload;
					Serialization::serializeToJson(m_clientInfo, clientInfoPayload);

					// Construct a disconnect server message for the queue
					json disconnectRequestJson;
					disconnectRequestJson["requestType"] = "disconnect";
					disconnectRequestJson["requestId"]= -1;
					disconnectRequestJson["version"] = 0;
					disconnectRequestJson["payload"] = clientInfoPayload;

					m_functionCallQueue->enqueue(disconnectRequestJson.dump());
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
						MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage")
							<< "Received unsupported binary message";
					}
				} break;
			case ix::WebSocketMessageType::Error:
				{
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") 
						<< "Error: " << msg->errorInfo.reason;
				}
				break;
			case ix::WebSocketMessageType::Ping:
				{
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") << "Ping";
				}
				break;
			case ix::WebSocketMessageType::Pong:
				{
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") << "Pong";
				}
				break;
			case ix::WebSocketMessageType::Fragment:
				{
					MIKAN_MT_LOG_ERROR("WebSocketClientConnection::handleClientMessage") << "Fragment";
				}
				break;
		}
	}

	template <typename t_message_type>
	bool sendSimpleMessage()
	{
		t_message_type event = {};
		json eventJson = event;
		std::string eventJsonString = eventJson.dump();

		return sendText(eventJsonString);
	}

	bool sendText(const std::string& textData)
	{
		WebSocketPtr websocket = m_websocket.lock();

		if (websocket)
		{
			auto sendInfo = websocket->sendText(textData);

			return sendInfo.success;
		}

		return false;
	}

	bool sendBinaryData(const std::vector<uint8_t>& binaryData)
	{
		WebSocketPtr websocket = m_websocket.lock();

		if (websocket)
		{
			ix::IXWebSocketSendData sendData(binaryData);
			auto sendInfo = websocket->sendBinary(sendData);

			return sendInfo.success;
		}

		return false;
	}

protected: 
	bool parseClientInfo(const std::string& configJsonString, MikanClientInfo& outClientInfo)
	{
		try
		{
			Serialization::deserializeFromJsonString(configJsonString, outClientInfo);
		}
		catch (json::exception e)
		{
			MIKAN_MT_LOG_ERROR("WebSocketClientConnection::parseClientInfo") 
				<< "Failed to parse client info: " << e.what();
			return false;
		}

		return true;
	}

private:
	LockFreeRequestQueuePtr m_functionCallQueue;
	WebSocketWeakPtr m_websocket;
	MikanClientInfo	m_clientInfo;
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

		auto connectionStateFactory = []() -> WebSocketClientConnectionPtr {
			return std::make_shared<WebSocketClientConnection>();
		};

		auto clientConnectCallback = [this](
			WebSocketWeakPtr webSocketWeakPtr, 
			ConnectionStatePtr connectionState) 
		{
			WebSocketPtr webSocket = webSocketWeakPtr.lock();
			if (!webSocket)
				return;

			WebSocketClientConnectionPtr clientConnectionState = 
				std::static_pointer_cast<WebSocketClientConnection>(connectionState);

			// Bind the websocket to the connection state
			clientConnectionState->bindWebSocket(webSocketWeakPtr);

			// Bind the message handler to the connection
			webSocket->setOnMessageCallback([clientConnectionState](const ix::WebSocketMessagePtr& msg) {
				clientConnectionState->handleClientMessage(clientConnectionState, msg);
			});

			// Add the connection to the list of connections
			{
				std::lock_guard<std::mutex> lock(m_connectionsMutex);

				m_connections.push_back(clientConnectionState);
			}
		};

		m_server->setConnectionStateFactory(connectionStateFactory);
		m_server->setOnConnectionCallback(clientConnectCallback);
		
		// Start listening for connections
		std::pair<bool, std::string> result= m_server->listen();
		if (result.first)
		{
			m_server->start();
		}
		else
		{
			MIKAN_LOG_INFO("WebsocketInterprocessMessageServer::initialize()") 
				<< "Listen error: " << result.second;
			bSuccess = false;
		}
	}

	return bSuccess;
}

void WebsocketInterprocessMessageServer::dispose()
{
	if (m_server)
	{
		// Close down all connections
		std::vector<WebSocketClientConnectionPtr> connections;
		getConnectionList(connections);

		// Disconnect all clients
		for (WebSocketClientConnectionPtr connection : m_connections)
		{
			// Tell the client that they are getting disconnected
			MikanDisconnectedEvent disconnectEvent;
			disconnectEvent.eventType = MikanDisconnectedEvent::k_typeName;

			std::string eventJsonString;
			Serialization::serializeToJsonString(disconnectEvent, eventJsonString);

			connection->sendText(eventJsonString);

			// Close the connection
			connection->disconnect();
		}

		// Clear the connection list
		{
			std::lock_guard<std::mutex> lock(m_connectionsMutex);

			m_connections.clear();
		}

		// Give us 500ms for the server to notice that clients went away
		ThreadUtils::sleepMilliseconds(500);
		m_server->stop();
	}

	ix::uninitNetSystem();
}

void WebsocketInterprocessMessageServer::setRequestHandler(
	const std::string& functionName, 
	RequestHandler handler)
{
	m_requestHandlers[functionName] = handler;
}

void WebsocketInterprocessMessageServer::getConnectionList(std::vector<WebSocketClientConnectionPtr>& outConnections)
{
	std::lock_guard<std::mutex> lock(m_connectionsMutex);

	for (WebSocketClientConnectionPtr connection : m_connections)
	{
		if (connection)
		{
			outConnections.push_back(connection);
		}
	}
}

WebSocketClientConnectionPtr WebsocketInterprocessMessageServer::findConnection(const std::string& connectionId)
{
	std::lock_guard<std::mutex> lock(m_connectionsMutex);

	for (WebSocketClientConnectionPtr connection : m_connections)
	{
		if (connection && connection->getId() == connectionId)
		{
			return connection;
		}
	}

	return nullptr;
}

void WebsocketInterprocessMessageServer::sendMessageToClient(const std::string& connectionId, const std::string& message)
{
	WebSocketClientConnectionPtr connection= findConnection(connectionId);

	if (connection)
	{
		connection->sendText(message);
	}
}

void WebsocketInterprocessMessageServer::sendMessageToAllClients(const std::string& message)
{
	std::vector<WebSocketClientConnectionPtr> connections;
	getConnectionList(connections);

	for (WebSocketClientConnectionPtr connection : connections)
	{
		connection->sendText(message);
	}
}

void WebsocketInterprocessMessageServer::processRequests()
{
	std::vector<WebSocketClientConnectionPtr> connections;
	getConnectionList(connections);

	// Process all connections	
	for (WebSocketClientConnectionPtr connection : connections)
	{
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

			int version;
			JsonSaxIntegerValueSearcher versionSearcher;
			if (!versionSearcher.fetchKeyValuePair(inRequestString, "version", version) || 
				version < 0)
			{
				MIKAN_LOG_WARNING("processRequests") << 
					"Request missing/invalid version field: " << inRequestString;
				continue;
			}

			// Request ID is optional if the request doesn't expect a response
			int requestId;
			JsonSaxIntegerValueSearcher requestIdSearcher;
			if (!requestIdSearcher.fetchKeyValuePair(inRequestString, "requestId", requestId))
			{
				requestId= INVALID_MIKAN_ID;
			}

			// Get the response from a registered function handler, if any
			ClientResponse outResponse;
			auto handler_it = m_requestHandlers.find(requestType);
			if (handler_it != m_requestHandlers.end())
			{
				// NOTE: Connection ID here is a unique ID for the websocket connection on the server
				// and is not the same as the client ID that the client sends to identify itself
				const std::string connectionId = connection->getId();
				ClientRequest request= { connectionId, requestId, inRequestString };

				handler_it->second(request, outResponse);
			}
			else
			{
				MikanResponse outResult;
				outResult.responseType = MikanResponse::k_typeName;
				outResult.requestId= requestId;
				outResult.resultCode= MikanResult_UnknownFunction;

				Serialization::serializeToJsonString(outResult, outResponse.utf8String);
			}

			// Send the response back to the client
			if (!outResponse.utf8String.empty())
			{
				connection->sendText(outResponse.utf8String);
			}

			if (!outResponse.binaryData.empty())
			{
				connection->sendBinaryData(outResponse.binaryData);
			}

			if (requestId != INVALID_MIKAN_ID &&
				outResponse.utf8String.empty() &&
				outResponse.binaryData.empty())
			{
				MIKAN_LOG_WARNING("processRequests") <<
					"Request handler for " << requestType 
					<< " returned empty response, but response expected!";
			}
		}
	}
}