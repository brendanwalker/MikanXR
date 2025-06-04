#include "MikanSteamVRDeviceMesh.h"
#include "MikanSteamVRDevice.h"

MikanSteamVRDeviceMesh::MikanSteamVRDeviceMesh(
	MikanSteamVRDevice* ownerDevice,
	const std::string& componentName, 
	const std::string& renderModelName)
	: m_ownerDevice(ownerDevice)
	, m_componentName(componentName)
	, m_renderModelName(renderModelName)
{
}

MikanSteamVRDeviceMesh::~MikanSteamVRDeviceMesh()
{

}


const char* MikanSteamVRDeviceMesh::getName() const
{
	return m_componentName.c_str();
}

bool MikanSteamVRDeviceMesh::getRelativePose(VRDevicePose& outPose) const
{
	// TODO
	return m_ownerDevice->getDevicePose(outPose);
}

IMkTriangulatedMeshConstPtr MikanSteamVRDeviceMesh::getTriangulatedMesh() const
{
	return IMkTriangulatedMeshConstPtr();
}

IMkWireframeMeshConstPtr MikanSteamVRDeviceMesh::getWireframeMesh() const
{
	return IMkWireframeMeshConstPtr();
}