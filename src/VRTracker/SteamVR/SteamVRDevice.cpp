#include "App.h"
#include "GlStaticMeshInstance.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "SteamVRDevice.h"
#include "SteamVRDeviceEnumerator.h"
#include "SteamVRDeviceProperties.h"
#include "SteamVRManager.h"
#include "SteamVRRenderComponent.h"
#include "VRDeviceEnumerator.h"
#include "VRDeviceManager.h"

#include "openvr.h"

#include <glm/gtx/matrix_decompose.hpp>

SteamVRDevice::SteamVRDevice()
	: m_deviceProperties(nullptr)
	, m_devicePath("")
	, m_driverType(IVRDeviceInterface::eDriverType::SteamVR)
	, m_deviceType(eDeviceType::INVALID)
	, m_boundScene(nullptr)
	, m_vrTrackerState(new CommonVRDeviceState)
	, m_poseMatrix(glm::mat4(1.0f))
	, m_isPoseValid(false)
{
}

SteamVRDevice::~SteamVRDevice()
{
	delete m_vrTrackerState;

	if (getIsOpen())
	{
		MIKAN_LOG_ERROR("~SteamVRTracker") << "VRTracker deleted without calling close() first!";
	}
}

bool SteamVRDevice::open()
{
	SteamVRDeviceEnumerator enumerator;
	bool success = false;

	if (enumerator.isValid())
	{
		success = open(&enumerator);
	}

	return success;
}

void SteamVRDevice::updateProperties()
{
	if (m_deviceProperties == nullptr)
		return;

	// Fetch existing state to see what changes
	const std::string oldRenderModelName= m_deviceProperties->getRenderModelName();

	// Fetch all the device properties again
	m_deviceProperties->updateProperties();

	// If the render model name changed, rebuild the child render components
	if (m_deviceProperties->getRenderModelName() != oldRenderModelName)
	{
		rebuildRenderComponents();
	}
}

void SteamVRDevice::updatePose()
{
	if (!getIsOpen())
		return;

	const SteamVRManager* steamVRMgr = 
		App::getInstance()->getVRDeviceManager()->getSteamVRManager();
	const vr::TrackedDevicePose_t* newPose = 
		steamVRMgr->getDevicePose(m_deviceProperties->getSteamVRDeviceIndex());

	if (newPose != nullptr && newPose->bPoseIsValid)
	{
		setPoseMatrix(vr_HmdMatrix34_to_glm_mat4(newPose->mDeviceToAbsoluteTracking));
		m_isPoseValid = true;
	}
	else
	{
		setPoseMatrix(glm::mat4(1.0f));
		m_isPoseValid = false;
	}

	// Update render component poses now that the transform of the parent device has changed
	for (auto it = m_renderComponents.begin(); it != m_renderComponents.end(); ++it)
	{
		it->second->updateComponent();
	}
}

void SteamVRDevice::bindToScene(GlScene* scene)
{
	removeFromBoundScene();

	for (auto it = m_renderComponents.begin(); it != m_renderComponents.end(); ++it)
	{
		it->second->bindToScene(scene);
	}
	m_boundScene= scene;
}

void SteamVRDevice::removeFromBoundScene()
{
	for (auto it = m_renderComponents.begin(); it != m_renderComponents.end(); ++it)
	{
		it->second->removeFromBoundScene();
	}
	m_boundScene= nullptr;
}

bool SteamVRDevice::matchesDeviceEnumerator(const DeviceEnumerator* enumerator) const
{
	// Down-cast the enumerator so we can use the correct get_path.
	const VRDeviceEnumerator* pEnum = static_cast<const VRDeviceEnumerator*>(enumerator);

	return pEnum->getDevicePath() == m_devicePath;
}

bool SteamVRDevice::open(const DeviceEnumerator* enumerator)
{
	const VRDeviceEnumerator* vrDeviceEnumerator = static_cast<const VRDeviceEnumerator*>(enumerator);
	const SteamVRDeviceEnumerator* steamVRTrackerEnumerator = vrDeviceEnumerator->getSteamVRTrackerEnumerator();
	const char* devicePath = steamVRTrackerEnumerator->getDevicePath();
	const vr::TrackedDeviceIndex_t steamVRDeviceId = steamVRTrackerEnumerator->getSteamVRDeviceID();
	const eDeviceType deviceType = steamVRTrackerEnumerator->getDeviceType();

	bool bSuccess = true;

	if (getIsOpen())
	{
		MIKAN_LOG_WARNING("SteamVRTracker::open") << "SteamVRTracker(" << devicePath << ") already open. Ignoring request.";
	}
	else
	{

		MIKAN_LOG_INFO("SteamVRTracker::open") << "Opening SteamVRTracker(" << devicePath << ", steamVRDeviceId=" << steamVRDeviceId << ")";
		m_devicePath = devicePath;
		m_deviceType= deviceType;

		// Fetch the relevant device properties from the steam VR device
		m_deviceProperties = new SteamVRDeviceProperties(steamVRDeviceId);
		m_deviceProperties->updateProperties();

		// Generate render components using the component list from the properties
		rebuildRenderComponents();
	}

	if (!bSuccess)
	{
		close();
	}

	return bSuccess;
}

bool SteamVRDevice::getIsOpen() const
{
	return m_deviceProperties != nullptr;
}

void SteamVRDevice::close()
{
	m_devicePath = "";

	disposeRenderComponents();

	if (m_deviceProperties != nullptr)
	{
		delete m_deviceProperties;
		m_deviceProperties = nullptr;
	}
}

eDeviceType SteamVRDevice::getDeviceType() const
{
	return m_deviceType;
}

std::string SteamVRDevice::getDevicePath() const
{
	return m_devicePath;
}

std::string SteamVRDevice::getSerialNumber() const
{
	return m_deviceProperties->getSerialNumber();
}

std::string SteamVRDevice::getTrackerRole() const
{
	return m_deviceProperties->getTrackingRole();
}

bool SteamVRDevice::getIsPoseValid() const
{
	return m_isPoseValid;
}

glm::quat SteamVRDevice::getOrientation() const
{
	return m_vrTrackerState->TrackingSpaceOrientation;
}

glm::vec3 SteamVRDevice::getPosition() const
{
	return m_vrTrackerState->TrackingSpacePosition;
}

const CommonVRDeviceState* SteamVRDevice::getVRTrackerState()
{
	return m_vrTrackerState;
}

void SteamVRDevice::rebuildRenderComponents()
{
	// Clean up any previous render components
	disposeRenderComponents();

	// Create render components for the device
	if (m_deviceProperties != nullptr)
	{
		auto& componentNames = m_deviceProperties->getRenderComponentNames();

		for (auto it = componentNames.begin(); it != componentNames.end(); ++it)
		{
			auto& componentInfo = *it;

			SteamVRRenderComponent* renderComponent =
				new SteamVRRenderComponent(
					this,
					componentInfo.componentName,
					componentInfo.renderModelName,
					componentInfo.isRenderable);

			renderComponent->initComponent();
			renderComponent->updateComponent();
			if (m_boundScene != nullptr)
			{
				renderComponent->bindToScene(m_boundScene);
			}

			m_renderComponents.insert({ componentInfo.componentName, renderComponent });
		}
	}
}

void SteamVRDevice::disposeRenderComponents()
{
	for (auto it = m_renderComponents.begin(); it != m_renderComponents.end(); ++it)
	{
		SteamVRRenderComponent* renderComponent = it->second;

		renderComponent->disposeComponent();
		delete renderComponent;
	}
	m_renderComponents.clear();
}

void SteamVRDevice::setPoseMatrix(const glm::mat4& mat)
{
	m_poseMatrix = mat;

	m_vrTrackerState->TrackingSpaceOrientation= glm::quat_cast(mat);
	m_vrTrackerState->TrackingSpacePosition= mat[3];
}

bool SteamVRDevice::getComponentPoseByName(const std::string& componentName, glm::mat4& outPose) const
{
	auto it= m_renderComponents.find(componentName);
	if (it != m_renderComponents.end())
	{
		outPose= it->second->getComponentPoseMatrix();
		return true;
	}

	return false;
}

void SteamVRDevice::getComponentNames(std::vector<std::string>& outComponentName) const
{
	for (auto it = m_renderComponents.begin(); it != m_renderComponents.end(); ++it)
	{
		outComponentName.push_back(it->first);
	}
}