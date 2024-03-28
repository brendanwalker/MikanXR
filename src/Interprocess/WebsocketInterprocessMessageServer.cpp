#include "WebsocketInterprocessMessageServer.h"
#include "JsonUtils.h"
#include "MikanEventTypes.h"
#include "MikanEventTypes_json.h"
#include "MikanAPITypes_json.h"
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

	const std::string getClientId() const { return m_clientInfo.clientId; }
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

						if (it.first == "clientInfo")
						{
							json clientInfoJson;
							if (parseClientInfo(msg, clientInfoJson))
							{
								// Remember the client info for this connection
								m_clientInfo= clientInfoJson;

								// Construct a connect server message for the queue
								json connectRequestJson;
								connectRequestJson["requestType"]= "connect";
								connectRequestJson["version"]= 0;
								connectRequestJson["payload"]= clientInfoJson;

								m_functionCallQueue->enqueue(connectRequestJson.dump());
							}
						}
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

					// Construct a disconnect server message for the queue
					json disconnectRequestJson;
					disconnectRequestJson["requestType"] = "disconnect";
					disconnectRequestJson["version"] = 0;
					disconnectRequestJson["payload"] = m_clientInfo;

					m_functionCallQueue->enqueue(disconnectRequestJson);
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

	template <typename t_message_type>
	bool sendSimpleMessage()
	{
		t_message_type event = {};
		json eventJson = event;
		std::string eventJsonString = eventJson.dump();

		return sendMessage(eventJsonString);
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

protected: 
	bool parseClientInfo(const ix::WebSocketMessagePtr& msg, json& outClientInfoPayload)
	{
		try
		{
			outClientInfoPayload = json::parse(msg->str);
		}
		catch (json::exception e)
		{
			MIKAN_MT_LOG_ERROR("ClientConnectionState::parseClientInfo") 
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
		}
	}
}