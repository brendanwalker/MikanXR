#include "CameraComponent.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"
#include "VRDeviceComponent.h"
#include "VRObjectSystem.h"
#include "VRDeviceMath.h"

// -- VRDevicePoseView -----
VRDevicePoseView::VRDevicePoseView()
	: m_deviceComponent()
	, m_poseSpace(eVRDevicePoseSpace::INVALID)
	, m_socketName("")
{
}

VRDevicePoseView::VRDevicePoseView(
	const VRDeviceComponent* deviceComponent,
	eVRDevicePoseSpace space,
	const std::string& socketName)
	: m_deviceComponent(deviceComponent->getSelfWeakPtr<const VRDeviceComponent>())
	, m_poseSpace(space)
	, m_socketName(socketName)
{
}

VRDevicePoseViewPtr VRDevicePoseView::makePoseView(
	const VRDeviceComponent* deviceComponent,
	eVRDevicePoseSpace space,
	const std::string& socketName)
{
	return std::make_shared<VRDevicePoseView>(deviceComponent, space, socketName);
}

VRDevicePoseViewPtr VRDevicePoseView::makeInvalidPoseView()
{
	return std::make_shared<VRDevicePoseView>();
}

const VRDeviceComponent* VRDevicePoseView::getDeviceComponent() const
{
	return m_deviceComponent.lock().get();
}

bool VRDevicePoseView::getPose(
	CameraComponentConstPtr cameraContext,
	glm::mat4& outXformInSpace) const
{
	// Use the tracking frame delay from the camera context
	const VRDeviceComponent* deviceComponent = getDeviceComponent();
	const int vrFrameDelay = cameraContext->getCameraDefinition()->getTrackingFrameDelay();

	VRDevicePose vrDevicePose;
	if (deviceComponent != nullptr &&
		deviceComponent->getDevicePose(vrFrameDelay, vrDevicePose))
	{
		// Get the VR Tracking Space transform of the device
		glm::mat4 vrDeviceXform = VRDevicePose_to_GlmTransform(vrDevicePose).getMat4();

		// Get the relative socket pose (default to identity xform if unavailable)
		glm::mat4 relativeSocketXform = glm::mat4(1.f);
		if (!m_socketName.empty())
		{
			deviceComponent->getSocketRelativePoseByName(m_socketName, relativeSocketXform);
		}
		else
		{
			deviceComponent->getDefaultSocketRelativePose(relativeSocketXform);
		}

		// Compute the vr tracking space transform of the socket
		glm::mat4 vrTrackingSpaceXform = glm_composite_xform(relativeSocketXform, vrDeviceXform);

		// Convert the VR Tracking Space Xform into the desired tracking space
		if (m_poseSpace == eVRDevicePoseSpace::MikanScene)
		{			
			auto config = cameraContext->getObjectSystemOfType<VRObjectSystem>()->getVRSystemConfigConst();
			const glm::mat4 glmVRDevicePoseOffset = MikanMatrix4f_to_glm_mat4(config->getVRDevicePoseOffset());

			// Convert the vr tracking space pose to Mikan scene space
			outXformInSpace = glm_composite_xform(vrTrackingSpaceXform, glmVRDevicePoseOffset);
		}
		else
		{
			// Return the vr tracking space pose
			outXformInSpace = vrTrackingSpaceXform;
		}

		return true;
	}

	return false;
}

bool VRDevicePoseView::getPose(
	CameraComponentConstPtr cameraContext,
	glm::dmat4& outPoseInSpace) const
{
	glm::mat4 poseInSpace;

	if (getPose(cameraContext, poseInSpace))
	{
		outPoseInSpace = glm::dmat4(poseInSpace);
		return true;
	}

	return false;
}