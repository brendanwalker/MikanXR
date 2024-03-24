#pragma once

#include "InterprocessMessages.h"

#include <map>
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
	WebsocketInterprocessMessageServer();
	virtual ~WebsocketInterprocessMessageServer();

	bool initialize() override;
	void dispose() override;
	void setRequestHandler(const std::string& requestType, RequestHandler handler, int version= 0) override;

	void sendMessageToClient(const std::string& clientId, const std::string& message) override;
	void sendMessageToAllClients(const std::string& message) override;
	void processRequests() override;

protected:
	static std::string makeRequestHandlerKey(const std::string& requestType, int version);

private:
	WebSocketServerPtr m_server;
	std::vector<ClientConnectionStateWeakPtr> m_connections;
	std::map<std::string, RequestHandler> m_requestHandlers;
};


#pragma once
