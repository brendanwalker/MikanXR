#pragma once

#include "IVRDevice.h"
#include "MikanSteamVRDeviceFwd.h"

#include <string>

class MikanSteamVRDeviceSocket : public IVRDeviceSocket
{
public:
	MikanSteamVRDeviceSocket(
		MikanSteamVRDevice* ownerDevice,
		const std::string& socketName);
	virtual ~MikanSteamVRDeviceSocket();

	virtual const char* getName() const override;
	virtual bool getRelativePose(VRDevicePose& outPose) const override;

private:
	MikanSteamVRDevice* m_ownerDevice = nullptr;
	std::string m_socketName;
};