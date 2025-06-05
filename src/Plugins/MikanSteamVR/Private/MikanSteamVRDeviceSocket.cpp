#include "MikanSteamVRDeviceSocket.h"
#include "MikanSteamVRDevice.h"
#include "SteamVRDeviceProperties.h"
#include "MikanSteamVRMath.h"

#include "openvr.h"

MikanSteamVRDeviceSocket::MikanSteamVRDeviceSocket(
	MikanSteamVRDevice* ownerDevice,
	const std::string& socketName)
	: m_ownerDevice(ownerDevice)
	, m_socketName(socketName)
{
}

const char* MikanSteamVRDeviceSocket::getName() const
{
	return m_socketName.c_str();
}

bool MikanSteamVRDeviceSocket::getSocketState(VRDevicePose& outRelativePose) const
{
	vr::IVRRenderModels* renderModelInterface = vr::VRRenderModels();
	const std::string ownerRenderModelName =
		m_ownerDevice->getProperties()->getRenderModelName();

	vr::VRControllerState_t* controllerState= nullptr;
	vr::RenderModel_ComponentState_t* componentState= nullptr;
	vr::RenderModel_ControllerMode_State_t* componentModeState= nullptr;

	if (renderModelInterface != nullptr &&
		renderModelInterface->GetComponentState(
			ownerRenderModelName.c_str(),
			m_socketName.c_str(),
			controllerState,
			componentModeState,
			componentState))
	{
		const glm::mat4 componentToRenderMat =
			vr_HmdMatrix34_to_glm_mat4(componentState->mTrackingToComponentLocal);

		outRelativePose = glm_mat4_to_VRDevicePose(componentToRenderMat);
		return true;
	}

	return false;
}
