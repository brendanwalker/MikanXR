#include "CameraRequestHandler.h"
#include "MikanServer.h"
#include "MikanCameraEvents.h"
#include "ServerResponseHelpers.h"

bool CameraRequestHandler::startup(MainWindow* mainWindow)
{
	return true;
}

// Camera Events
void CameraRequestHandler::publishCameraNewFrameEvent(const MikanCameraNewFrameEvent& newFrameEvent)
{
	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(newFrameEvent));
}