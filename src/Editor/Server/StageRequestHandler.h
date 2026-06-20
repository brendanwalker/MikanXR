#pragma once

#include "IServerRequestHandler.h"

class StageRequestHandler : public IServerRequestHandler
{
public:
	StageRequestHandler(class MikanServer* owner)
		: IServerRequestHandler(owner)
	{
	}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
};