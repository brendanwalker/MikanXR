#pragma once

#include "ObjectSystemConfigFwd.h"

class MikanServer;
struct ClientRequest;
struct ClientResponse;

class IServerRequestHandler
{
public:
	IServerRequestHandler(class MikanServer* owner) : m_owner(owner) {}
	virtual ~IServerRequestHandler() {}

	virtual bool startup(class MainWindow* mainWindow) = 0;
	virtual void shutdown() = 0;

	ProjectConfigPtr getProjectConfig();

protected:
	MikanServer* m_owner;
};