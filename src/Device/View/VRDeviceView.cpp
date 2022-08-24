//-- includes -----
#include "App.h"
#include "ProfileConfig.h"
#include "SteamVRDevice.h"
#include "VRDeviceView.h"
#include "VRDeviceEnumerator.h"

#include "glm/gtc/quaternion.hpp"

//-- public implementation -----
VRDeviceView::VRDeviceView(const int device_id)
	: DeviceView(device_id)
	, m_device(nullptr)
{
}

VRDeviceView::~VRDeviceView()
{
	if (m_device != nullptr)
	{
		delete m_device;
	}
}

eDeviceType VRDeviceView::getVRDeviceType() const
{
	return m_device->getDeviceType();
}

IVRDeviceInterface::eDriverType VRDeviceView::getVRTrackerDriverType() const
{
	return m_device->getDriverType();
}

std::string VRDeviceView::getDevicePath() const
{
	return m_device->getDevicePath();
}

std::string VRDeviceView::getSerialNumber() const
{
	return m_device->getSerialNumber();
}

std::string VRDeviceView::getTrackerRole() const
{
	return m_device->getTrackerRole();
}

bool VRDeviceView::open(const class DeviceEnumerator* enumerator)
{
	bool bSuccess = DeviceView::open(enumerator);

	return bSuccess;
}

void VRDeviceView::close()
{
	DeviceView::close();
}

bool VRDeviceView::allocateDeviceInterface(const class DeviceEnumerator* enumerator)
{
	m_device = VRDeviceView::allocateVRTrackerInterface(enumerator);

	return m_device != nullptr;
}

IVRDeviceInterface* VRDeviceView::allocateVRTrackerInterface(const DeviceEnumerator* enumerator)
{
	const VRDeviceEnumerator* vrTrackerEnumerator = static_cast<const VRDeviceEnumerator*>(enumerator);
	IVRDeviceInterface* tracker_interface = nullptr;

	switch (vrTrackerEnumerator->getVRTrackerApi())
	{
	case eVRTrackerDeviceApi::STEAMVR:
	{
		tracker_interface = new SteamVRDevice();
	} break;
	default:
		break;
	}

	return tracker_interface;
}

void VRDeviceView::freeDeviceInterface()
{
	if (m_device != nullptr)
	{
		delete m_device;  // Deleting abstract object should be OK because
		// this (ServerDeviceView) is abstract as well.
		// All non-abstract children will have non-abstract types
		// for m_device.
		m_device = nullptr;
	}
}

bool VRDeviceView::getIsPoseValid() const
{
	return m_device != nullptr ? m_device->getIsPoseValid() : false;
}

glm::quat VRDeviceView::getOrientation() const
{
	return m_device != nullptr ? m_device->getOrientation() : glm::quat();
}

glm::vec3 VRDeviceView::getPosition() const
{
	return m_device != nullptr ? m_device->getPosition() : glm::vec3(0.f, 0.f, 0.f);
}

bool VRDeviceView::getComponentPoseByName(const std::string& componentName, glm::mat4& outPose) const
{
	return m_device != nullptr ? m_device->getComponentPoseByName(componentName, outPose) : false;
}

void VRDeviceView::getComponentNames(std::vector<std::string>& outComponentName) const
{
	if (m_device != nullptr)
	{
		m_device->getComponentNames(outComponentName);
	}
}

glm::mat4 VRDeviceView::getCalibrationPose() const
{
	const ProfileConfig* config= App::getInstance()->getProfileConfig();
	const std::string& componentName = config->calibrationComponentName;

	glm::mat4 componentPose= glm::mat4(1.f);
	if (!getComponentPoseByName(componentName, componentPose))
	{
		if (getIsPoseValid())
		{
			const glm::vec3 devicePos = getPosition();
			const glm::quat deviceQuat = getOrientation();

			componentPose = glm::translate(glm::mat4(1.0), devicePos) * glm::mat4_cast(deviceQuat);
		}
	}

	return componentPose;
}