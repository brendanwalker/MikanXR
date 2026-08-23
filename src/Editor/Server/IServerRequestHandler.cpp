#include "App.h"
#include "IServerRequestHandler.h"
#include "MikanServer.h"
#include "MainWindow.h"

double IServerRequestHandler::getServerTime() const
{
	auto* ownerWindow= m_owner->getOwnerWindow();
	App* ownerApp= ownerWindow ? ownerWindow->getOwnerApp() : nullptr;

	return ownerApp ? ownerApp->getSecondsSinceAppStart() : 0.0;
}

ProjectManagerPtr IServerRequestHandler::getProjectManager() const
{
	auto* ownerWindow= m_owner->getOwnerWindow();

	return ownerWindow ? ownerWindow->getProjectManager() : ProjectManagerPtr();
}

ProjectConfigPtr IServerRequestHandler::getProjectConfig() const { return m_owner->getProjectConfig(); }
