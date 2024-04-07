#include "MikanVRDeviceAPI.h"
#include "MikanVRDeviceTypes_json.h"
#include "MikanRequestManager.h"

MikanVRDeviceAPI::MikanVRDeviceAPI(MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{
	m_requestManager->addResponseFactory<MikanVRDeviceList>();
	m_requestManager->addResponseFactory<MikanVRDeviceInfo>();
}

MikanResponseFuture MikanVRDeviceAPI::getVRDeviceList() // returns MikanVRDeviceList
{
	return m_requestManager->sendRequest(k_getVRDeviceList);
}

MikanResponseFuture MikanVRDeviceAPI::getVRDeviceInfo(MikanVRDeviceID deviceId) // returns MikanVRDeviceInfo
{
	return m_requestManager->sendRequestWithPayload<int>(k_getVRDeviceInfo, deviceId);
}

MikanResponseFuture MikanVRDeviceAPI::subscribeToVRDevicePoseUpdates(MikanVRDeviceID deviceId) // returns MikanResponse
{
	return m_requestManager->sendRequestWithPayload<int>(k_subscribeToVRDevicePoseUpdates, deviceId);
}

MikanResponseFuture MikanVRDeviceAPI::unsubscribeFromVRDevicePoseUpdates(MikanVRDeviceID deviceId) // returns MikanResponse
{
	return m_requestManager->sendRequestWithPayload<int>(k_unsubscribeFromVRDevicePoseUpdates, deviceId);
}
