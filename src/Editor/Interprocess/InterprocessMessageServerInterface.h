#pragma once

#include <functional>
#include <string>

#define WEBSOCKET_SERVER_ADDRESS			"ws://127.0.0.1"
#define WEBSOCKET_SERVER_PORT				"8080"
#define WEBSOCKET_PROTOCOL_PREFIX			"Mikan-"

#define WEBSOCKET_CONNECT_EVENT				"connect"
#define WEBSOCKET_DISCONNECT_EVENT			"disconnect"
#define WEBSOCKET_ERROR_EVENT				"error"
#define WEBSOCKET_PING_EVENT				"ping"
#define WEBSOCKET_PONG_EVENT				"pong"

struct ClientSocketEvent
{
	std::string connectionId;
	std::string eventType;
	std::vector<std::string> eventArgs;
};

using MikanRequestID = int;

struct ClientRequest
{
	std::string connectionId;
	MikanRequestID requestId;
	std::string utf8RequestString;
};

struct ClientResponse
{
	std::string utf8String;
	std::vector<uint8_t> binaryData;
};

using SocketEventHandler = std::function<void(const ClientSocketEvent& event)>;
using RequestHandler = std::function<void(const ClientRequest& request, ClientResponse& response)>;

class IInterprocessMessageServer
{
public:
	virtual ~IInterprocessMessageServer() {}

	virtual bool initialize() = 0;
	virtual void dispose() = 0;
	virtual void setSocketEventHandler(const std::string& eventType, SocketEventHandler handler) = 0;
	virtual void setRequestHandler(std::size_t requestTypeId, RequestHandler handler) = 0;

	virtual void sendMessageToClient(const std::string& connectionId, const std::string& message) = 0;
	virtual void sendMessageToAllClients(const std::string& message) = 0;
	virtual void processSocketEvents() = 0;
	virtual void processRequests() = 0;
};