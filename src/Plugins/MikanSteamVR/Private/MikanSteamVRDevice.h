#pragma once

#include "IVRDevice.h"
#include "MikanSteamVRDeviceFwd.h"

#include <string>
#include <map>

class MikanSteamVRDevice : public IVRDevice
{
public:
	MikanSteamVRDevice(class MikanSteamVRManager* ownerDeviceManager, vr::TrackedDeviceIndex_t steamvrDeviceId);
	virtual ~MikanSteamVRDevice();

	// -- Device Properties
	virtual size_t getDeviceIndex() const override;
	virtual eVRDeviceType getDeviceType() const override;
	virtual const char* getDevicePath() const override;
	virtual bool getDevicePose(int vrFrameDelay, VRDevicePose& outPose) const override;
	virtual const char* getSerialNumber() const override;
	virtual const char* getTrackingSystem() const override;
	virtual const char* getTrackerRole() const override;
	virtual const char* getModelLabel() const override;
	virtual const char* getModelNumber() const override;
	virtual const char* getManufacturerName() const override;

	// -- Socket accessors
	virtual size_t getSocketCount() const override;
	virtual IVRDeviceSocket* geSocketByIndex(size_t socketIndex) const override;
	virtual IVRDeviceSocket* getSocketByName(const char* socketName) const override;

	// -- Mesh accessors
	virtual size_t getMeshCount() const override;
	virtual IVRDeviceMesh* getMeshByIndex(size_t meshIndex) const override;
	virtual IVRDeviceMesh* getMeshByName(const char* meshName) const override;

	// -- SteamVR
	inline class MikanSteamVRManager* getOwnerDeviceManager() { return m_ownerDeviceManager; }
	inline class SteamVRDeviceProperties* getProperties() const { return m_deviceProperties; }
	void updateProperties();

protected:
	void rebuildComponents();
	void disposeComponents();

private:
	class MikanSteamVRManager* m_ownerDeviceManager;
	class SteamVRDeviceProperties* m_deviceProperties;
	std::map<std::string, MikanSteamVRDeviceSocketPtr> m_sockets;
	std::map<std::string, MikanSteamVRDeviceMeshPtr> m_meshes;
};
