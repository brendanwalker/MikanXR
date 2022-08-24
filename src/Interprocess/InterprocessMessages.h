#pragma once

#include "MikanClientTypes.h"

#include <functional>
#include <map>
#include <string>

#include <stdint.h>

#include <boost/interprocess/interprocess_fwd.hpp>

#define FUNCTION_CALL_QUEUE_NAME			"MikanFunctionCallQueue"
#define SERVER_EVENT_QUEUE_PREFIX			"MikanServerEventQueue_"
#define FUNCTION_RESPONSE_QUEUE_PREFIX		"MikanFunctionResponseQueue_"

#define CONNECT_FUNCTION_NAME				"connect"
#define DISCONNECT_FUNCTION_NAME			"disconnect"

class MikanRemoteFunctionCall
{
public:
	MikanRemoteFunctionCall();
	MikanRemoteFunctionCall(const char* clientId, const char* functionName);
	MikanRemoteFunctionCall(const char* clientId, const char* functionName, uint8_t* buffer, size_t bufferSize);

	void setClientId(const char* clientId);
	void setFunctionName(const char* functionName);
	void setParameterBuffer(uint8_t* buffer, size_t bufferSize);

	const char* getClientId() const { return m_header.clientId; }
	const char* getFunctionName() const { return m_header.functionName; }
	uint32_t getRequestId() const { return m_header.requestId; }
	size_t getTotalSize() const;

	template <typename t_parameter_type>
	bool extractParameters(t_parameter_type& outParameter) const
	{
		if (sizeof(t_parameter_type) == m_header.parameterBufferSize)
		{
			memcpy(&outParameter, m_parameterBuffer, m_header.parameterBufferSize);
			return true;
		}

		return false;
	}

private:
	struct
	{
		char clientId[64];
		char functionName[64];
		uint32_t requestId;
		size_t parameterBufferSize;
	} m_header;
	uint8_t m_parameterBuffer[1024];

	static uint32_t m_nextRequestId;
};

class MikanRemoteFunctionResult
{
public:
	MikanRemoteFunctionResult();
	MikanRemoteFunctionResult(MikanResult result, uint32_t requestId);
	MikanRemoteFunctionResult(MikanResult result, uint32_t requestId, uint8_t* buffer, size_t bufferSize);

	void setResultCode(MikanResult result);
	void setRequestId(uint32_t requestId);
	void setResultBuffer(uint8_t* buffer, size_t bufferSize);

	MikanResult getResultCode() const { return m_header.resultCode; }
	uint32_t getRequestId() const { return m_header.requestId; }
	size_t getTotalSize() const;

	template <typename t_result_type>
	bool extractResult(t_result_type& outResult) const
	{
		if (sizeof(t_result_type) == m_header.resultBufferSize)
		{
			memcpy(&outResult, m_resultBuffer, m_header.resultBufferSize);
			return true;
		}

		return false;
	}

private:
	struct
	{
		MikanResult resultCode;
		uint32_t requestId;
		size_t resultBufferSize;
	} m_header;
	uint8_t m_resultBuffer[2048];
};

class InterprocessMessageClient
{
public:
	InterprocessMessageClient();
	~InterprocessMessageClient();

	MikanResult connect(const std::string& clientId, MikanClientInfo* client);
	void disconnect();

	bool tryFetchNextServerEvent(MikanEvent* outEvent);
	MikanResult callRemoteFunction(const MikanRemoteFunctionCall* inFunctionCall, MikanRemoteFunctionResult* outResult);
	MikanResult callRemoteFunction(const char* functionName, MikanRemoteFunctionResult* outResult);
	MikanResult callRemoteFunction(const char* functionName, uint8_t* buffer, size_t bufferSize, MikanRemoteFunctionResult* outResult);

	const std::string& getClientId() const { return m_clientId; }
	const MikanClientInfo& getClientInfo() const { return m_clientInfo; }
	const bool getIsConnected() const { return m_isConnected; }

private:
	std::string m_clientId;
	MikanClientInfo m_clientInfo;
	std::string m_serverEventQueueName;
	std::string m_functionResponseQueueName;
	boost::interprocess::message_queue* m_severEventQueue;
	boost::interprocess::message_queue* m_functionCallQueue;
	boost::interprocess::message_queue* m_functionResponseQueue;
	bool m_isConnected;
};

class InterprocessMessageConnection
{
public:
	InterprocessMessageConnection();
	~InterprocessMessageConnection();

	bool initialize(const std::string& clientId);
	void dispose();

	const std::string getClientId() const { return m_clientId; }

	bool sendEvent(MikanEvent* event);
	bool sendFunctionResponse(MikanRemoteFunctionResult* result);

private:
	std::string m_clientId;
	std::string m_serverEventQueueName;
	std::string m_functionResponseQueueName;
	boost::interprocess::message_queue* m_eventQueue;
	boost::interprocess::message_queue* m_functionResponseQueue;
};

class InterprocessMessageServer
{
public:
	using RPCHandler = std::function<void(const MikanRemoteFunctionCall* inFunctionCall, MikanRemoteFunctionResult* outResult)>;

	InterprocessMessageServer();
	~InterprocessMessageServer();

	bool initialize();
	void dispose();
	void setRPCHandler(const std::string& functionName, RPCHandler handler);

	void sendServerEventToClient(const std::string& clientId, MikanEvent* event);
	void sendServerEventToAllClients(MikanEvent* event);
	void processRemoteFunctionCalls();

private:
	boost::interprocess::message_queue* m_functionCallQueue;
	std::map<std::string, InterprocessMessageConnection*> m_connections;
	std::map<std::string, RPCHandler> m_functionHandlers;
};

