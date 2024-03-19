#pragma once

#include "InterprocessMessages.h"

#include <memory>

namespace ix
{
	class WebSocketServer;
	class ConnectionState;
}
using WebSocketServerPtr = std::shared_ptr<ix::WebSocketServer>;
using ConnectionStatePtr = std::shared_ptr<ix::ConnectionState>;
using ClientConnectionStatePtr = std::shared_ptr<class ClientConnectionState>;
using ClientConnectionStateWeakPtr = std::weak_ptr<class ClientConnectionState>;

class WebsocketInterprocessMessageServer : public IInterprocessMessageServer
{
public:
	using RPCHandler = std::function<void(const MikanRemoteFunctionCall* inFunctionCall, MikanRemoteFunctionResult* outResult)>;

	WebsocketInterprocessMessageServer();
	virtual ~WebsocketInterprocessMessageServer();

	bool initialize() override;
	void dispose() override;
	void setRPCHandler(const std::string& functionName, RPCHandler handler) override;

	void sendServerEventToClient(const std::string& clientId, MikanEvent* event) override;
	void sendServerEventToAllClients(MikanEvent* event) override;
	void processRemoteFunctionCalls() override;

private:
	WebSocketServerPtr m_server;
	std::vector<ClientConnectionStateWeakPtr> m_connections;
	std::map<std::string, RPCHandler> m_functionHandlers;
};


#pragma once
