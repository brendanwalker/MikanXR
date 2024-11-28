#pragma once

#include "InterprocessMessages.h"

#include <map>
#include <memory>
#include <mutex>

namespace ix
{
	class WebSocketServer;
	class ConnectionState;
}
using WebSocketServerPtr = std::shared_ptr<ix::WebSocketServer>;
using ConnectionStatePtr = std::shared_ptr<ix::ConnectionState>;
using WebSocketClientConnectionPtr = std::shared_ptr<class WebSocketClientConnection>;
using WebsocketClientEventQueuePtr = std::shared_ptr<class WebsocketClientEventQueue>;

class WebsocketInterprocessMessageServer : public IInterprocessMessageServer
{
public:
	WebsocketInterprocessMessageServer();
	virtual ~WebsocketInterprocessMessageServer();

	inline WebsocketClientEventQueuePtr getClientDisconnectEventQueue() { return m_clientDisconnectEventQueue; }

	bool initialize() override;
	void dispose() override;
	void setRequestHandler(uint64_t requestTypeId, RequestHandler handler) override;
	void setClientDisconnectHandler(ClientDisconnectHandler handler) override;

	void sendMessageToClient(const std::string& connectionId, const std::string& message) override;
	void sendMessageToAllClients(const std::string& message) override;
	void processRequests() override;
	void processDisconnections() override;

protected:
	void getConnectionList(std::vector<WebSocketClientConnectionPtr>& outConnections);
	WebSocketClientConnectionPtr findConnection(const std::string& clientId);

private:
	WebSocketServerPtr m_server;
	std::vector<WebSocketClientConnectionPtr> m_connections;
	std::mutex m_connectionsMutex;
	std::map<uint64_t, RequestHandler> m_requestHandlers;
	ClientDisconnectHandler m_clientDisconnectHandler;
	WebsocketClientEventQueuePtr m_clientDisconnectEventQueue;
};


#pragma once
