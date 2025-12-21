#include "IServerRequestHandler.h"
#include "MikanServer.h"
#include "MainWindow.h"

ProjectManagerPtr IServerRequestHandler::getProjectManager() const
{
	return m_owner->getOwnerWindow()->getProjectManager();
}

ProjectConfigPtr IServerRequestHandler::getProjectConfig() const
{
	return m_owner->getProjectConfig();
}
