#include "MikanSteamVRDevice.h"
#include "MikanSteamVRManager.h"
#include "MikanSteamVRMath.h"
#include "MikanSteamVRDeviceMesh.h"
#include "MikanSteamVRDeviceSocket.h"
#include "SteamVRDeviceProperties.h"

#include "openvr.h"

MikanSteamVRDevice::MikanSteamVRDevice(
	MikanSteamVRManager* ownerDeviceManager,
	vr::TrackedDeviceIndex_t steamvrDeviceId)
	: m_ownerDeviceManager(ownerDeviceManager)
	, m_deviceProperties(new SteamVRDeviceProperties(steamvrDeviceId))
{
}

MikanSteamVRDevice::~MikanSteamVRDevice()
{
	delete m_deviceProperties;
}

size_t MikanSteamVRDevice::getDeviceIndex() const
{
	return m_deviceProperties->getSteamVRDeviceIndex();
}

eVRDeviceType MikanSteamVRDevice::getDeviceType() const
{
	switch (m_deviceProperties->getSteamVRDeviceClass())
	{
		case vr::TrackedDeviceClass_HMD:
			return eVRDeviceType::HMD;
		case vr::TrackedDeviceClass_Controller:
			return eVRDeviceType::VRController;
		case vr::TrackedDeviceClass_GenericTracker:
			return eVRDeviceType::VRTracker;
		default:
			return eVRDeviceType::INVALID;
	}
}

const char* MikanSteamVRDevice::getDevicePath() const
{
	return m_deviceProperties->getDevicePath().c_str();
}

bool MikanSteamVRDevice::getDevicePose(int vrFrameDelay, VRDevicePose& outPose) const
{
	const vr::TrackedDevicePose_t* rawDevicePose=
		m_ownerDeviceManager->getDevicePose(
			m_deviceProperties->getSteamVRDeviceIndex(), vrFrameDelay);

	if (rawDevicePose != nullptr)
	{
		// Convert the raw pose to our VRDevicePose format
		glm::mat4 xform= vr_HmdMatrix34_to_glm_mat4(rawDevicePose->mDeviceToAbsoluteTracking);
		outPose = glm_mat4_to_VRDevicePose(xform);

		return true;
	}

	return false;
}

const char* MikanSteamVRDevice::getSerialNumber() const
{
	return m_deviceProperties->getSerialNumber().c_str();
}

const char* MikanSteamVRDevice::getTrackingSystem() const
{
	return m_deviceProperties->getTrackingSystem().c_str();
}

const char* MikanSteamVRDevice::getTrackerRole() const
{
	return m_deviceProperties->getTrackingRole().c_str();
}

const char* MikanSteamVRDevice::getModelLabel() const
{
	return m_deviceProperties->getModelLabel().c_str();
}

const char* MikanSteamVRDevice::getModelNumber() const
{
	return m_deviceProperties->getModelNumber().c_str();
}

const char* MikanSteamVRDevice::getManufacturerName() const
{
	return m_deviceProperties->getManufacturerName().c_str();
}

size_t MikanSteamVRDevice::getSocketCount() const
{
	return m_sockets.size();
}

IVRDeviceSocket* MikanSteamVRDevice::geSocketByIndex(size_t socketIndex) const
{
	if (socketIndex < m_sockets.size())
	{
		auto it = m_sockets.begin();
		std::advance(it, socketIndex);
		return it->second.get();
	}
	return nullptr;
}

IVRDeviceSocket* MikanSteamVRDevice::getSocketByName(const char* socketName) const
{
	auto it = m_sockets.find(socketName);
	if (it != m_sockets.end())
	{
		return it->second.get();
	}
	return nullptr;

}

size_t MikanSteamVRDevice::getMeshCount() const
{
	return m_meshes.size();
}

IVRDeviceMesh* MikanSteamVRDevice::getMeshByIndex(size_t meshIndex) const
{
	if (meshIndex < m_meshes.size())
	{
		auto it = m_meshes.begin();
		std::advance(it, meshIndex);
		return it->second.get();
	}
	return nullptr;
}

IVRDeviceMesh* MikanSteamVRDevice::getMeshByName(const char* meshName) const
{
	auto it = m_meshes.find(meshName);
	if (it != m_meshes.end())
	{
		return it->second.get();
	}
	return nullptr;
}

void MikanSteamVRDevice::updateProperties()
{
	// Fetch existing state to see what changes
	const std::string oldRenderModelName = m_deviceProperties->getRenderModelName();

	// Fetch all the device properties again
	m_deviceProperties->updateProperties();

	// If the render model name changed, rebuild the child render components
	if (m_deviceProperties->getRenderModelName() != oldRenderModelName)
	{
		rebuildComponents();
	}
}

void MikanSteamVRDevice::rebuildComponents()
{
	// Clean up any previous render components
	disposeComponents();

	// Create render components for the device
	if (m_deviceProperties != nullptr)
	{
		for (const auto& definition : m_deviceProperties->getRenderComponentDefinitions())
		{
			if (definition.isRenderable)
			{
				m_meshes.insert({
					definition.componentName, 
					std::make_shared<MikanSteamVRDeviceMesh>(
						this,
						definition.componentName,
						definition.renderModelName)});
			}
			else
			{
				m_sockets.insert({
					definition.componentName,
					std::make_shared<MikanSteamVRDeviceSocket>(
						this,
						definition.componentName)});
			}
		}
	}
}

void MikanSteamVRDevice::disposeComponents()
{
	m_meshes.clear();
	m_sockets.clear();
}