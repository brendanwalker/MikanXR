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

class IInterprocessMessageClient
{
public:
	using ResponseHandler = std::function<void(const std::string& utf8ResponseString)>;

	virtual ~IInterprocessMessageClient() {}

	virtual MikanResult initialize() = 0;
	virtual void dispose() = 0;

	virtual MikanResult setClientProperty(const std::string& key, const std::string& value) = 0;
	virtual void setResponseHandler(ResponseHandler handler) = 0;

	virtual MikanResult connect(const std::string& host, const std::string& port) = 0;
	virtual void disconnect() = 0;

	virtual MikanResult fetchNextEvent(
		size_t utf8BufferSize,
		char* outUtf8Buffer,
		size_t* outUtf8BufferSizeNeeded) = 0;
	virtual MikanResult sendRequest(const std::string& utf8RequestString) = 0;

	virtual const std::string& getClientId() const = 0;
	virtual const bool getIsConnected() const = 0;
};

class IInterprocessMessageServer
{
public:
	using RequestHandler = std::function<void(const std::string& utf8RequestString, std::string& utf8ResponseString)>;

	virtual ~IInterprocessMessageServer() {}

	virtual bool initialize() = 0;
	virtual void dispose() = 0;
	virtual void setRequestHandler(const std::string& requestType, RequestHandler handler, int version= 0) = 0;

	virtual void sendMessageToClient(const std::string& clientId, const std::string& message) = 0;
	virtual void sendMessageToAllClients(const std::string& message) = 0;
	virtual void processRequests() = 0;
};