#pragma once

#include "IServerRequestHandler.h"
#include "CommonConfigFwd.h"

class ShapeRequestHandler : public IServerRequestHandler
{
public:
	ShapeRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;

protected:
	// Shape Requests
	void getModelShapeRenderGeometryHandler(const struct ClientRequest& request, struct ClientResponse& response);
};
