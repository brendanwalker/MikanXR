#pragma once

#include "IServerRequestHandler.h"
#include "MikanTypeFwd.h"

#include <set>

class VRDeviceClientState
{
public:
	VRDeviceClientState()= default;
	VRDeviceClientState(class MikanClientConnectionState* owner);

	void subscribeToVRDevicePoseUpdatesHandler(MikanVRDeviceID deviceId);
	void unsubscribeFromVRDevicePoseUpdatesHandler(MikanVRDeviceID deviceId);
	void publishVRDevicePoses(int64_t newFrameIndex);

protected:
	class MikanClientConnectionState* m_owner= nullptr;
	std::set<MikanVRDeviceID> m_subscribedVRDevices;
};

class VRDeviceRequestHandler : public IServerRequestHandler
{
public:
	VRDeviceRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
	// VRDevice Events
	void publishVRDeviceListChanged();
	void publishVRDevicePoses(int64_t newFrameIndex);

	// VRDevice Requests
	void getVRDeviceListHandler(const ClientRequest& request, ClientResponse& response);
	void getVRDeviceInfoHandler(const ClientRequest& request, ClientResponse& response);
	void subscribeToVRDevicePoseUpdatesHandler(const ClientRequest& request, ClientResponse& response);
	void unsubscribeFromVRDevicePoseUpdatesHandler(const ClientRequest& request, ClientResponse& response);
};