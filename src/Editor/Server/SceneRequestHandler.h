#pragma once

#include "IServerRequestHandler.h"

class SceneRequestHandler : public IServerRequestHandler
{
public:
	SceneRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
};