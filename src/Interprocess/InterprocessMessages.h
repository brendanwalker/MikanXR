#pragma once

#include "MikanCoreTypes.h"

#include <functional>
#include <string>

#define WEBSOCKET_SERVER_ADDRESS			"ws://127.0.0.1"
#define WEBSOCKET_SERVER_PORT				"8080"

#define WEBSOCKET_CONNECT_EVENT				"connect"
#define WEBSOCKET_DISCONNECT_EVENT			"disconnect"
#define WEBSOCKET_ERROR_EVENT				"error"
#define WEBSOCKET_PING_EVENT				"ping"
#define WEBSOCKET_PONG_EVENT				"pong"

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
using RequestHandler = std::function<void(const ClientRequest& request, ClientResponse& response)>;
class IInterprocessMessageClient
{
public:
	using TextResponseHandler = std::function<void(const std::string& utf8ResponseString)>;
	using BinaryResponseHandler = std::function<void(const uint8_t* buffer, size_t bufferSize)>;

	virtual ~IInterprocessMessageClient() {}

	virtual MikanResult initialize() = 0;
	virtual void dispose() = 0;

	virtual void setTextResponseHandler(TextResponseHandler handler) = 0;
	virtual void setBinaryResponseHandler(BinaryResponseHandler handler) = 0;

	virtual MikanResult connect(const std::string& host, const std::string& port) = 0;
	virtual void disconnect() = 0;

	virtual MikanResult fetchNextEvent(
		size_t utf8BufferSize,
		char* outUtf8Buffer,
		size_t* outUtf8BufferSizeNeeded) = 0;
	virtual MikanResult sendRequest(const std::string& utf8RequestString) = 0;

	virtual const bool getIsConnected() const = 0;
};

class IInterprocessMessageServer
{
public:
	using ClientDisconnectHandler = std::function<void(const std::string& connectionId)>;

	virtual ~IInterprocessMessageServer() {}

	virtual bool initialize() = 0;
	virtual void dispose() = 0;
	virtual void setRequestHandler(uint64_t requestTypeId, RequestHandler handler) = 0;
	virtual void setClientDisconnectHandler(ClientDisconnectHandler handler) = 0;

	virtual void sendMessageToClient(const std::string& connectionId, const std::string& message) = 0;
	virtual void sendMessageToAllClients(const std::string& message) = 0;
	virtual void processRequests() = 0;
	virtual void processDisconnections() = 0;
};