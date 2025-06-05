#include "IMkStaticMeshInstance.h"
#include "IMkTriangulatedMesh.h"
#include "MikanSteamVRDeviceMesh.h"
#include "MikanSteamVRDevice.h"
#include "MikanSteamVRMath.h"
#include "MikanSteamVRManager.h"
#include "SteamVRDeviceProperties.h"
#include "SteamVRRenderModelResource.h"
#include "SteamVRResourceManager.h"
#include "StringUtils.h"

#include "openvr.h"

MikanSteamVRDeviceMesh::MikanSteamVRDeviceMesh(
	MikanSteamVRDevice* ownerDevice,
	const std::string& componentName, 
	const std::string& renderModelName)
	: m_ownerDevice(ownerDevice)
	, m_componentName(componentName)
	, m_renderModelName(renderModelName)
{
	MikanSteamVRManager* ownerDeviceManager = m_ownerDevice->getOwnerDeviceManager();
	auto& resourceManager = ownerDeviceManager->getResourceManager();
	SteamVRRenderModelResource* modelResource = resourceManager->fetchRenderModel(m_renderModelName);

	if (modelResource != nullptr)
	{
		m_triangulatedMesh = modelResource->getTriangulatedMesh();
		m_wireframeMesh = modelResource->getWireframeMesh();
	}
}

const char* MikanSteamVRDeviceMesh::getName() const
{
	return m_componentName.c_str();
}

bool MikanSteamVRDeviceMesh::getMeshState(
	VRDevicePose& outRelativePose, 
	bool& outIsVisible) const
{
	vr::IVRRenderModels* renderModelInterface = vr::VRRenderModels();
	const std::string ownerRenderModelName =
		m_ownerDevice->getProperties()->getRenderModelName();

	vr::VRControllerState_t* m_controllerState = nullptr;
	vr::RenderModel_ComponentState_t* m_componentState = nullptr;
	vr::RenderModel_ControllerMode_State_t* m_componentModeState = nullptr;

	if (renderModelInterface != nullptr &&
		renderModelInterface->GetComponentState(
			ownerRenderModelName.c_str(),
			m_componentName.c_str(),
			m_controllerState,
			m_componentModeState,
			m_componentState))
	{
		const glm::mat4 trackingToRenderMat =
			vr_HmdMatrix34_to_glm_mat4(m_componentState->mTrackingToComponentRenderModel);

		outRelativePose = glm_mat4_to_VRDevicePose(trackingToRenderMat);
		outIsVisible = (m_componentState->uProperties & (uint32_t)vr::VRComponentProperty_IsVisible) != 0;

		return true;
	}

	return false;
}

IMkTriangulatedMeshConstPtr MikanSteamVRDeviceMesh::getTriangulatedMesh() const
{
	return m_triangulatedMesh;
}

IMkWireframeMeshConstPtr MikanSteamVRDeviceMesh::getWireframeMesh() const
{
	return m_wireframeMesh;
}