#pragma once

#include "IServerRequestHandler.h"

class RemoteControlRequestHandler : public IServerRequestHandler
{
public:
	RemoteControlRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup() override;

protected:
	void gotoAppStageHandler(const ClientRequest& request, ClientResponse& response);
	void getAppStageInfoHandler(const ClientRequest& request, ClientResponse& response);
};