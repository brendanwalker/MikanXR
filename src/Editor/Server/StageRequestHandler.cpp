#include "App.h"
#include "AppStage.h"
#include "StageRequestHandler.h"
#include "MainWindow.h"
#include "MikanServer.h"
#include "ServerResponseHelpers.h"

#include <functional>

using namespace std::placeholders;

// -- StageRequestHandler -- //
bool StageRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer= m_owner->getMessageServer();

	// Register remote control request handlers

	return true;
}

void StageRequestHandler::shutdown() {}