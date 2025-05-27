#pragma once

#include "IServerRequestHandler.h"
#include "CommonConfigFwd.h"

class StencilRequestHandler : public IServerRequestHandler
{
public:
	StencilRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
	// Stencil Events
	void handleStencilSystemConfigChange(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	// Stencil Requests
	void getQuadStencilListHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getQuadStencilHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getBoxStencilListHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getBoxStencilHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getModelStencilListHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getModelStencilHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getModelStencilRenderGeometryHandler(const struct ClientRequest& request, struct ClientResponse& response);
};