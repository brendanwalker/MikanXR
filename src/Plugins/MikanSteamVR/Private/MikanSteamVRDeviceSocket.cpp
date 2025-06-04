#include "MikanSteamVRDeviceSocket.h"
#include "MikanSteamVRDevice.h"

MikanSteamVRDeviceSocket::MikanSteamVRDeviceSocket(
	MikanSteamVRDevice* ownerDevice,
	const std::string& socketName)
	: m_ownerDevice(ownerDevice)
	, m_socketName(socketName)
{

}

MikanSteamVRDeviceSocket::~MikanSteamVRDeviceSocket()
{}

const char* MikanSteamVRDeviceSocket::getName() const
{
	return m_socketName.c_str();
}

bool MikanSteamVRDeviceSocket::getRelativePose(VRDevicePose& outPose) const
{
	return false;
}
