#pragma once

#include "IServerRequestHandler.h"

class CameraRequestHandler : public IServerRequestHandler
{
public:
	CameraRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override {}

	void publishCameraNewFrameEvent(const struct MikanCameraNewFrameEvent& newFrameEvent);
	void publishCameraAttachmentChangedEvent();

protected:
	void getCameraListHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void findCameraByNameHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getCameraInfoHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getCameraAttachmentHandler(const struct ClientRequest& request, struct ClientResponse& response);
};