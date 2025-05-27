#pragma once

#include "CommonConfigFwd.h"
#include "IServerRequestHandler.h"

class AnchorRequestHandler : public IServerRequestHandler
{
public:
	AnchorRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
	// Anchor Events
	void handleAnchorSystemConfigChange(
		CommonConfigPtr configPtr,
		const class ConfigPropertyChangeSet& changedPropertySet);

	// Anchor Request Handler
	void getSpatialAnchorListHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getSpatialAnchorInfoHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void findSpatialAnchorInfoByNameHandler(const struct ClientRequest& request, struct ClientResponse& response);
};