//-- includes -----
#include "App.h"
#include "MathTypeConversion.h"
#include "MathGLM.h"
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
	return m_device ? m_device->getDeviceType() : eDeviceType::INVALID;
}

IVRDeviceInterface::eDriverType VRDeviceView::getVRTrackerDriverType() const
{
	return m_device ? m_device->getDriverType() : IVRDeviceInterface::eDriverType::INVALID;
}

std::string VRDeviceView::getDevicePath() const
{
	return m_device ? m_device->getDevicePath() : "";
}

std::string VRDeviceView::getSerialNumber() const
{
	return m_device ? m_device->getSerialNumber() : "";
}

std::string VRDeviceView::getTrackerRole() const
{
	return m_device ? m_device->getTrackerRole() : "";
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

void VRDeviceView::getComponentNames(std::vector<std::string>& outComponentName) const
{
	if (m_device != nullptr)
	{
		m_device->getComponentNames(outComponentName);
	}
}

bool VRDeviceView::getComponentPoseByName(
	const std::string& componentName, 
	bool bApplyVRDeviceOffset,
	glm::mat4& outPose) const
{
	glm::mat4 rawComponentPose = glm::mat4(1.f);

	if (m_device != nullptr &&
		m_device->getComponentPoseByName(componentName, rawComponentPose))
	{
		outPose = bApplyVRDeviceOffset ? applyVRDeviceOffset(rawComponentPose) : rawComponentPose;

		return true;
	}

	return false;
}

glm::mat4 VRDeviceView::getDefaultComponentPose(bool bApplyVRDeviceOffset) const
{
	glm::mat4 resultPose = glm::mat4(1.f);
	if (m_device != nullptr)
	{
		ProfileConfigConstPtr config = App::getInstance()->getProfileConfig();

		std::string componentName= "";
		if (m_device->getDeviceType() == eDeviceType::VRTracker)
		{
			componentName = config->vivePuckDefaultComponentName;
		}

		if (!getComponentPoseByName(componentName, bApplyVRDeviceOffset, resultPose))
		{
			const glm::vec3 devicePos = m_device->getPosition();
			const glm::quat deviceQuat = m_device->getOrientation();
			const glm::mat4 rawDevicePose= glm_composite_xform(
				glm::mat4_cast(deviceQuat), 
				glm::translate(glm::mat4(1.0), devicePos));

			resultPose = bApplyVRDeviceOffset ? applyVRDeviceOffset(rawDevicePose) : rawDevicePose;
		}
	}

	return resultPose;
}

glm::mat4 VRDeviceView::applyVRDeviceOffset(const glm::mat4& rawDevicePose)
{
	ProfileConfigConstPtr config = App::getInstance()->getProfileConfig();
	const glm::mat4 glmVRDevicePoseOffset = MikanMatrix4f_to_glm_mat4(config->vrDevicePoseOffset);

	return glm_composite_xform(rawDevicePose, glmVRDevicePoseOffset);
}