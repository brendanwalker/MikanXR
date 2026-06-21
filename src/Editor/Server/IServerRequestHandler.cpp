#include "IServerRequestHandler.h"
#include "MikanServer.h"
#include "MainWindow.h"

ProjectManagerPtr IServerRequestHandler::getProjectManager() const
{
	auto* ownerWindow= m_owner->getOwnerWindow();

	return ownerWindow ? ownerWindow->getProjectManager() : ProjectManagerPtr();
}

ProjectConfigPtr IServerRequestHandler::getProjectConfig() const { return m_owner->getProjectConfig(); }
