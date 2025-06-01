#pragma once

#include "IServerRequestHandler.h"
#include "IVRDeviceManager.h"
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

class VRDeviceRequestHandler : public IServerRequestHandler, public IVRDeviceManagerListener
{
public:
	VRDeviceRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
	// IVRDeviceManagerListener
	virtual void onActiveDeviceListChanged() override;
	virtual void onDevicePropertyChanged(int deviceId) override;
	virtual void onDevicePosesChanged(int64_t newFrameId) override;

	// VRDevice Requests
	void getVRDeviceListHandler(const ClientRequest& request, ClientResponse& response);
	void getVRDeviceInfoHandler(const ClientRequest& request, ClientResponse& response);
	void subscribeToVRDevicePoseUpdatesHandler(const ClientRequest& request, ClientResponse& response);
	void unsubscribeFromVRDevicePoseUpdatesHandler(const ClientRequest& request, ClientResponse& response);
};