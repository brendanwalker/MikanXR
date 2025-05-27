#include "App.h"
#include "AppStage.h"
#include "SceneRequestHandler.h"
#include "MainWindow.h"
#include "MikanServer.h"
#include "MikanSceneEvents.h"
#include "MikanSceneRequests.h"
#include "ServerResponseHelpers.h"

#include <functional>

using namespace std::placeholders;

// -- SceneRequestHandler -- //
bool SceneRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	// Register remote control request handlers

	return true;
}

void SceneRequestHandler::shutdown()
{

}