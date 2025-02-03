//-- includes -----
#include "App.h"
#include "MathTypeConversion.h"
#include "MathGLM.h"
#include "ProfileConfig.h"
#include "SteamVRDevice.h"
#include "VRDeviceView.h"
#include "VRDeviceEnumerator.h"

#include "glm/gtc/quaternion.hpp"

// -- VRDeviceView -----
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
	glm::mat4& outPose) const
{
	glm::mat4 vrTrackingSpacePose = glm::mat4(1.f);

	if (m_device != nullptr &&
		m_device->getComponentPoseByName(componentName, vrTrackingSpacePose))
	{
		outPose = vrTrackingSpacePose;
		return true;
	}

	return false;
}

bool VRDeviceView::getDefaultComponentPose(glm::mat4& outPose) const
{
	bool bIsPoseValid = false;

	if (m_device != nullptr &&
		m_device->getDeviceType() == eDeviceType::VRTracker)
	{
		ProfileConfigConstPtr config = App::getInstance()->getProfileConfig();

		if (config)
		{
			// Get the vive puck default pose in VR Tracking space
			bIsPoseValid= getComponentPoseByName(
				config->vivePuckDefaultComponentName,
				outPose);
		}

		// Fallback to the device pose if the vive puck devault pose is not valid
		if (!bIsPoseValid)
		{
			const glm::vec3 devicePos = m_device->getPosition();
			const glm::quat deviceQuat = m_device->getOrientation();

			outPose = glm_composite_xform(
				glm::mat4_cast(deviceQuat),
				glm::translate(glm::mat4(1.0), devicePos));
			bIsPoseValid = true;
		}
	}

	return bIsPoseValid;
}

VRDevicePoseViewPtr VRDeviceView::makePoseView(
	eVRDevicePoseSpace space, 
	const std::string& subComponentName) const
{
	return std::make_shared<VRDevicePoseView>(this, space, subComponentName);
}

// -- VRDevicePoseView -----
VRDevicePoseView::VRDevicePoseView(
	const VRDeviceView* deviceView,
	eVRDevicePoseSpace space,
	const std::string& subComponentName)
	: m_deviceView(deviceView->getSelfWeakPtr<VRDeviceView>())
	, m_poseSpace(space)
	, m_subComponentName(subComponentName)
{
}

const VRDeviceView* VRDevicePoseView::getDeviceView() const
{
	return m_deviceView.lock().get();
}

bool VRDevicePoseView::getIsPoseValid() const
{
	const VRDeviceView* deviceView = getDeviceView();

	return deviceView != nullptr && deviceView->getIsPoseValid();
}

bool VRDevicePoseView::getPose(glm::mat4& outPoseInSpace) const
{
	bool bIsPoseValid = false;
	const VRDeviceView* deviceView = getDeviceView();

	if (deviceView != nullptr)
	{
		// Get the raw device pose
		glm::mat4 vrTrackingSpacePose = glm::mat4(1.f);
		if (!m_subComponentName.empty())
		{
			bIsPoseValid= deviceView->getComponentPoseByName(m_subComponentName, vrTrackingSpacePose);
		}
		else
		{
			bIsPoseValid = deviceView->getDefaultComponentPose(vrTrackingSpacePose);
		}

		if (bIsPoseValid)
		{
			if (m_poseSpace == eVRDevicePoseSpace::MikanScene)
			{
				ProfileConfigConstPtr config = App::getInstance()->getProfileConfig();
				const glm::mat4 glmVRDevicePoseOffset = MikanMatrix4f_to_glm_mat4(config->vrDevicePoseOffset);

				// Convert the vr tracking space pose to Mikan scene space
				outPoseInSpace = glm_composite_xform(vrTrackingSpacePose, glmVRDevicePoseOffset);
			}
			else
			{
				// Return the vr tracking space pose
				outPoseInSpace = vrTrackingSpacePose;
			}
		}
	}

	return bIsPoseValid;
}

bool VRDevicePoseView::getPose(glm::dmat4& outPoseInSpace) const
{
	glm::mat4 poseInSpace;

	if (getPose(poseInSpace))
	{
		outPoseInSpace = glm::dmat4(poseInSpace);
		return true;
	}

	return false;
}