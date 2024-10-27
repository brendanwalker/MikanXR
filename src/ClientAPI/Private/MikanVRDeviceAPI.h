#pragma once

#include "MikanAPIExport.h"
#include "MikanTypeFwd.h"
#include "MikanAPI.h"

class MikanVRDeviceAPI : public IMikanVRDeviceAPI
{
public:
	MikanVRDeviceAPI() = default;
	MikanVRDeviceAPI(class MikanRequestManager* requestManager);

	inline static const std::string k_getVRDeviceList = "getVRDeviceList";
	MikanResponseFuture getVRDeviceList(); // returns MikanVRDeviceList

	inline static const std::string k_getVRDeviceInfo = "getVRDeviceInfo";
	MikanResponseFuture getVRDeviceInfo(MikanVRDeviceID deviceId); // returns MikanVRDeviceInfo

	inline static const std::string k_subscribeToVRDevicePoseUpdates = "subscribeToVRDevicePoseUpdates";
	MikanResponseFuture subscribeToVRDevicePoseUpdates(MikanVRDeviceID deviceId); // returns MikanResponse

	inline static const std::string k_unsubscribeFromVRDevicePoseUpdates = "unsubscribeFromVRDevicePoseUpdates";
	MikanResponseFuture unsubscribeFromVRDevicePoseUpdates(MikanVRDeviceID deviceId); // returns MikanResponse

private:
	class MikanRequestManager* m_requestManager = nullptr;
};
