#pragma once

#include "InterprocessMessages.h"

#include <boost/interprocess/interprocess_fwd.hpp>

class BoostInterprocessMessageConnection
{
public:
	BoostInterprocessMessageConnection();
	~BoostInterprocessMessageConnection();

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

class BoostInterprocessMessageServer : public IInterprocessMessageServer
{
public:
	using RPCHandler = std::function<void(const MikanRemoteFunctionCall* inFunctionCall, MikanRemoteFunctionResult* outResult)>;

	BoostInterprocessMessageServer();
	virtual ~BoostInterprocessMessageServer();

	bool initialize() override;
	void dispose() override;
	void setRPCHandler(const std::string& functionName, RPCHandler handler) override;

	void sendServerEventToClient(const std::string& clientId, MikanEvent* event) override;
	void sendServerEventToAllClients(MikanEvent* event) override;
	void processRemoteFunctionCalls() override;

private:
	boost::interprocess::message_queue* m_functionCallQueue;
	std::map<std::string, BoostInterprocessMessageConnection*> m_connections;
	std::map<std::string, RPCHandler> m_functionHandlers;
};


