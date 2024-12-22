#pragma once

#include "InterprocessMessages.h"

#include <map>
#include <memory>
#include <mutex>
#include <string>

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

	bool initialize() override;
	void dispose() override;
	void setSocketEventHandler(const std::string& eventType, SocketEventHandler handler) override;
	void setRequestHandler(std::size_t requestTypeId, RequestHandler handler) override;

	void sendMessageToClient(const std::string& connectionId, const std::string& message) override;
	void sendMessageToAllClients(const std::string& message) override;
	void processSocketEvents() override;
	void processRequests() override;

protected:
	void getConnectionList(std::vector<WebSocketClientConnectionPtr>& outConnections);
	WebSocketClientConnectionPtr findConnection(const std::string& clientId);

private:
	WebSocketServerPtr m_server;
	std::vector<WebSocketClientConnectionPtr> m_connections;
	std::mutex m_connectionsMutex;
	std::map<std::string, SocketEventHandler> m_socketEventHandlers;
	std::map<std::size_t, RequestHandler> m_requestHandlers;
};


#pragma once
