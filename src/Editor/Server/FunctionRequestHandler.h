#pragma once

#include "CommonConfigFwd.h"
#include "ComponentFwd.h"
#include "IServerRequestHandler.h"
#include "ObjectSystemConfigFwd.h"

class FunctionRequestHandler : public IServerRequestHandler
{
public:
	FunctionRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
	// Function Events
	void onComponentFunctionsChanged(MikanObjectSystemConstPtr ownerSystem, MikanComponentConstPtr ownerComponent);

	// Function Request Handlers
	void invokeComponentFunctionRequestHandler(const ClientRequest& request, ClientResponse& response);
	void invokeSystemFunctionRequestHandler(const ClientRequest& request, ClientResponse& response);
	void getFunctionListHandler(const ClientRequest& request, ClientResponse& response);
};