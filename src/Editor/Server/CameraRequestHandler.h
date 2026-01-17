#pragma once

#include "IServerRequestHandler.h"

class CameraRequestHandler : public IServerRequestHandler
{
public:
	CameraRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override {}

	void publishCameraNewFrameEvent(const struct MikanCameraNewFrameEvent& newFrameEvent);
};