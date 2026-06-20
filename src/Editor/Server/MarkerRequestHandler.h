#pragma once

#include "IServerRequestHandler.h"

class MarkerRequestHandler : public IServerRequestHandler
{
public:
	MarkerRequestHandler(class MikanServer* owner)
		: IServerRequestHandler(owner)
	{
	}

	virtual bool startup(class MainWindow* mainWindow) override;

protected:
	void getArucoMarkerImageHandler(const ClientRequest& request, ClientResponse& response);
};
