#pragma once

#include "MikanCoreTypes.h"

#include <functional>
#include <string>

#define FUNCTION_CALL_QUEUE_NAME			"MikanFunctionCallQueue"
#define SERVER_EVENT_QUEUE_PREFIX			"MikanServerEventQueue_"
#define FUNCTION_RESPONSE_QUEUE_PREFIX		"MikanFunctionResponseQueue_"

#define CONNECT_FUNCTION_NAME				"connect"
#define DISCONNECT_FUNCTION_NAME			"disconnect"

#define WEBSOCKET_SERVER_ADDRESS			"ws://127.0.0.1"
#define WEBSOCKET_SERVER_PORT				"8080"

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

	virtual MikanResult setClientInfo(const std::string& clientInfo) = 0;
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
	virtual ~IInterprocessMessageServer() {}

	virtual bool initialize() = 0;
	virtual void dispose() = 0;
	virtual void setRequestHandler(const std::string& requestType, RequestHandler handler, int version= 0) = 0;

	virtual void sendMessageToClient(const std::string& connectionId, const std::string& message) = 0;
	virtual void sendMessageToAllClients(const std::string& message) = 0;
	virtual void processRequests() = 0;
};