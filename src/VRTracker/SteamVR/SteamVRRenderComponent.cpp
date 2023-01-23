#include "App.h"
#include "GlMaterialInstance.h"
#include "GlStaticMeshInstance.h"
#include "GlScene.h"
#include "MathTypeConversion.h"
#include "SteamVRDevice.h"
#include "SteamVRManager.h"
#include "SteamVRDeviceProperties.h"
#include "SteamVRRenderComponent.h"
#include "SteamVRRenderModelResource.h"
#include "SteamVRResourceManager.h"
#include "StringUtils.h"
#include "VRDeviceManager.h"

#include "glm/ext/matrix_float4x4.hpp"

#include "openvr.h"

SteamVRRenderComponent::SteamVRRenderComponent(
	SteamVRDevice* ownerDevice,
	const std::string& _componentName,
	const std::string& _renderModelName,
	bool _isRenderable)
	: m_ownerDevice(ownerDevice)
	, m_componentName(_componentName)
	, m_renderModelName(_renderModelName)
	, m_bIsRenderable(_isRenderable)
	, m_glMeshInstance(nullptr)
	, m_controllerState(new vr::VRControllerState_t)
	, m_componentState(new vr::RenderModel_ComponentState_t)
	, m_componentModeState(new vr::RenderModel_ControllerMode_State_t)
	, m_renderPoseMatrix(glm::mat4(1.f))
	, m_componentPoseMatrix(glm::mat4(1.f))
{
}

SteamVRRenderComponent::~SteamVRRenderComponent()
{
	delete m_controllerState;
	delete m_componentState;
	delete m_componentModeState;
}

bool SteamVRRenderComponent::initComponent()
{
	if (m_bIsRenderable)
	{
		std::unique_ptr<SteamVRResourceManager>& resourceManager=
			App::getInstance()->getVRDeviceManager()->getSteamVRManager()->getResourceManager();
		SteamVRRenderModelResource* modelResource =
			resourceManager->fetchRenderModel(m_renderModelName);

		if (modelResource != nullptr)
		{
			const vr::TrackedDeviceIndex_t deviceIndex =
				m_ownerDevice->getSteamVRDeviceProperties()->getSteamVRDeviceIndex();

			char szInstanceName[256];
			if (m_componentName.length() > 0)
			{
				StringUtils::formatString(
					szInstanceName,
					sizeof(szInstanceName),
					"SteamVRDevice_%d_%s_%s",
					deviceIndex,
					m_renderModelName.c_str(),
					m_componentName.c_str());
			}
			else
			{
				StringUtils::formatString(
					szInstanceName,
					sizeof(szInstanceName),
					"SteamVRDevice_%d_%s",
					deviceIndex,
					m_renderModelName.c_str());
			}

			m_glMeshInstance =
				new GlStaticMeshInstance(
					szInstanceName,
					modelResource->getTriangulatedMesh(),
					modelResource->getMaterial());

			return true;
		}

		return false;
	}
	else
	{
		// This component is just a simple transform
		return true;
	}
}

void SteamVRRenderComponent::updateComponent()
{
	// Get the transform of the parent device
	const glm::mat4 ownerPoseMat = m_ownerDevice->getPoseMatrix();
	bool bNewIsVisible= true;

	if (m_componentName.size() > 0)
	{
		// Bail if the render model interface isn't available
		vr::IVRRenderModels* renderModelInterface = vr::VRRenderModels();
		const std::string ownerRenderModelName=
			m_ownerDevice->getSteamVRDeviceProperties()->getRenderModelName();

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
			const glm::mat4 componentToRenderMat =
				vr_HmdMatrix34_to_glm_mat4(m_componentState->mTrackingToComponentLocal);

			m_renderPoseMatrix = ownerPoseMat * trackingToRenderMat;
			m_componentPoseMatrix = ownerPoseMat * componentToRenderMat;
			bNewIsVisible = (m_componentState->uProperties & (uint32_t)vr::VRComponentProperty_IsVisible) != 0;
		}
	}
	else 
	{
		m_renderPoseMatrix= ownerPoseMat;
		m_componentPoseMatrix= ownerPoseMat;
	}

	// Update pose and visibility on child mesh, if any
	if (m_glMeshInstance != nullptr)
	{
		m_glMeshInstance->setModelMatrix(m_renderPoseMatrix);
		m_glMeshInstance->setVisible(bNewIsVisible);
	}

}

void SteamVRRenderComponent::disposeComponent()
{	
	if (m_glMeshInstance != nullptr)
	{
		m_glMeshInstance->removeFromBoundScene();

		delete m_glMeshInstance;
		m_glMeshInstance= nullptr;
	}
}

void SteamVRRenderComponent::bindToScene(GlScene* scene)
{
	if (m_glMeshInstance != nullptr)
	{
		m_glMeshInstance->bindToScene(scene);
	}
}

void SteamVRRenderComponent::removeFromBoundScene()
{
	if (m_glMeshInstance != nullptr)
	{
		m_glMeshInstance->removeFromBoundScene();
	}
}

void SteamVRRenderComponent::setDiffuseColor(const glm::vec4& diffuseColor)
{
	if (m_glMeshInstance != nullptr)
	{
		m_glMeshInstance->getMaterialInstance()->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, diffuseColor);
	}
}