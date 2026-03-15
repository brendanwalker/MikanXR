#pragma once

#include "IServerRequestHandler.h"

class VideoSourceRequestHandler : public IServerRequestHandler
{
public:
	VideoSourceRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override {}

	// Video Source Events
	void publishVideoSourceOpenedEvent();
	void publishVideoSourceClosedEvent();
	void publishVideoSourceModeChangedEvent();

protected:
	void getVideoSourceIntrinsicsHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void getVideoSourceModeHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void setUSBVideoSourceDeviceHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void setUSBVideoSourceResolutionHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void setUSBVideoSourceFrameRateHandler(const struct ClientRequest& request, struct ClientResponse& response);
	void setUSBVideoSourceFormatHandler(const struct ClientRequest& request, struct ClientResponse& response);
};