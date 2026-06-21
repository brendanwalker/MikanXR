#pragma once

#include "IVRDevice.h"
#include "MikanSteamVRDeviceFwd.h"

#include <string>

class MikanSteamVRDeviceSocket : public IVRDeviceSocket
{
public:
	MikanSteamVRDeviceSocket(MikanSteamVRDevice* ownerDevice, const std::string& socketName);

	virtual const char* getName() const override;
	virtual bool getSocketState(VRDevicePose& outRelativePose) const override;

private:
	MikanSteamVRDevice* m_ownerDevice= nullptr;
	std::string m_socketName;
};