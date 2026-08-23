#pragma once

#include "CommonConfigFwd.h"
#include "ComponentFwd.h"
#include "IServerRequestHandler.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"

class FunctionRequestHandler : public IServerRequestHandler
{
public:
	FunctionRequestHandler(class MikanServer* owner)
		: IServerRequestHandler(owner)
	{
	}

	virtual bool startup(class MainWindow* mainWindow) override;

protected:
	// Function Request Handlers
	void invokeComponentFunctionRequestHandler(const ClientRequest& request, ClientResponse& response);
	void invokeSystemFunctionRequestHandler(const ClientRequest& request, ClientResponse& response);
	void getFunctionListHandler(const ClientRequest& request, ClientResponse& response);
};