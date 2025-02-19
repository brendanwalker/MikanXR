#pragma once

class MikanServer;
struct ClientRequest;
struct ClientResponse;

class IServerRequestHandler
{
public:
	IServerRequestHandler(class MikanServer* owner) : m_owner(owner) {}
	virtual ~IServerRequestHandler() {}

	virtual bool startup(class MainWindow* mainWindow)= 0;

protected:
	MikanServer* m_owner;
};