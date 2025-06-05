#pragma once

#include "IVRDevice.h"
#include "MikanSteamVRDeviceFwd.h"
#include <string>

class MikanSteamVRDeviceMesh : public IVRDeviceMesh
{
public:
	MikanSteamVRDeviceMesh(
		MikanSteamVRDevice* ownerDevice,
		const std::string& componentName, 
		const std::string& renderModelName);
	virtual ~MikanSteamVRDeviceMesh();

	virtual const char* getName() const override;
	virtual bool getMeshState(VRDevicePose& outRelativePose, bool& outIsVisible) const override;
	virtual IMkTriangulatedMeshConstPtr getTriangulatedMesh() const override;
	virtual IMkWireframeMeshConstPtr getWireframeMesh() const override;

private:
	MikanSteamVRDevice* m_ownerDevice = nullptr;
	std::string m_componentName;
	std::string m_renderModelName;
};
