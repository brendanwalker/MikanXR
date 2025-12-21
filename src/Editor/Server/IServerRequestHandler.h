#pragma once

#include "ObjectSystemConfigFwd.h"
#include "ProjectManager.h"

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

	ProjectManagerPtr getProjectManager() const;
	ProjectConfigPtr getProjectConfig() const;
	template <class t_object_system_type>
	std::shared_ptr<t_object_system_type> getObjectSystemOfType() const
	{
		return getProjectManager()->getSystemOfType<t_object_system_type>();
	}

protected:
	MikanServer* m_owner;
};