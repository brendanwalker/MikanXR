#pragma once

#include "IServerRequestHandler.h"
#include "CommonConfigFwd.h"

class StencilRequestHandler : public IServerRequestHandler
{
public:
	StencilRequestHandler(class MikanServer* owner)
		: IServerRequestHandler(owner)
	{
	}

	virtual bool startup(class MainWindow* mainWindow) override;

protected:
	// Stencil Requests
	void getModelStencilRenderGeometryHandler(const struct ClientRequest& request, struct ClientResponse& response);
};