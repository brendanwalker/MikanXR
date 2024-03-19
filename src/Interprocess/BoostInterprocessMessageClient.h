#pragma once

#include "InterprocessMessages.h"

#include <boost/interprocess/interprocess_fwd.hpp>

class BoostInterprocessMessageClient : public IInterprocessMessageClient
{
public:
	BoostInterprocessMessageClient();
	virtual ~BoostInterprocessMessageClient();

	MikanResult connect(const std::string& clientId, MikanClientInfo* client) override;
	void disconnect() override;

	bool tryFetchNextServerEvent(MikanEvent* outEvent) override;
	MikanResult callRemoteFunction(const MikanRemoteFunctionCall* inFunctionCall, MikanRemoteFunctionResult* outResult) override;
	MikanResult callRemoteFunction(const char* functionName, MikanRemoteFunctionResult* outResult) override;
	MikanResult callRemoteFunction(const char* functionName, uint8_t* buffer, size_t bufferSize, MikanRemoteFunctionResult* outResult) override;

	const std::string& getClientId() const override { return m_clientId; }
	const MikanClientInfo& getClientInfo() const override { return m_clientInfo; }
	const bool getIsConnected() const override { return m_isConnected; }

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