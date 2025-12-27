#include "App.h"
#include "Colors.h"
#include "IMkStaticMeshInstance.h"
#include "IMkTriangulatedMesh.h"
#include "IMkWireframeMesh.h"
#include "IVRDevice.h"
#include "MainWindow.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "MeshColliderComponent.h"
#include "MkMaterialInstance.h"
#include "MikanLineRenderer.h"
#include "MikanObject.h"
#include "MikanTextRenderer.h"
#include "MikanVRDeviceTypes.h"
#include "MulticastDelegate.h"
#include "ProjectConfig.h"
#include "StageComponent.h"
#include "SelectionComponent.h"
#include "StageObjectSystem.h"
#include "StaticMeshComponent.h"
#include "StringUtils.h"
#include "TransformComponent.h"
#include "VRDeviceComponent.h"
#include "VRObjectSystem.h"

// -- VRDeviceConfig -----
VRDeviceDefinition::VRDeviceDefinition(
	MikanVRDeviceID vrDeviceId,
	const std::string& vrDevicePath,
	const MikanTransform& xform)
	: TransformComponentDefinition(vrDeviceId, vrDevicePath, xform)
	, m_vrDeviceId(vrDeviceId)
	, m_vrDevicePath(vrDevicePath)
{}

// -- VRDeviceComponent -----
VRDeviceComponent::VRDeviceComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
	m_bWantsCustomRender = true;
}

void VRDeviceComponent::init()
{
	TransformComponent::init();

	// Create a selection component so that we can selection the mesh collision geometry
	SelectionComponentPtr selectionComponentPtr = getOwnerObject()->getComponentOfType<SelectionComponent>();
	if (selectionComponentPtr)
	{
		// Bind selection events
		selectionComponentPtr->OnInteractionRayOverlapEnter += MakeDelegate(this, &VRDeviceComponent::onInteractionRayOverlapEnter);
		selectionComponentPtr->OnInteractionRayOverlapExit += MakeDelegate(this, &VRDeviceComponent::onInteractionRayOverlapExit);
		selectionComponentPtr->OnInteractionSelected += MakeDelegate(this, &VRDeviceComponent::onInteractionSelected);
		selectionComponentPtr->OnInteractionUnselected += MakeDelegate(this, &VRDeviceComponent::onInteractionUnselected);

		// Remember the selection component
		m_selectionComponentWeakPtr = selectionComponentPtr;
	}

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
}

void VRDeviceComponent::setVRDeviceInterface(IVRDevice* vrDeviceInterface)
{
	// VRDevice interface should only be set before component is initialized
	assert(!m_bWasInitialized);
	m_vrDeviceInterface= vrDeviceInterface;
}

bool VRDeviceComponent::getDevicePose(const int vrFrameDelay, VRDevicePose& outPose) const
{
	if (m_vrDeviceInterface != nullptr)
	{
		return m_vrDeviceInterface->getDevicePose(vrFrameDelay, outPose);
	}

	return false;
}

void VRDeviceComponent::disposeSockets()
{
	// Clean up any previously created sockets
	for (auto& kvpair : m_socketMap)
	{
		TransformComponentPtr socketPtr = kvpair.second;
		socketPtr->dispose();
	}

	// Forget about the socket components
	m_socketMap.clear();
}

void VRDeviceComponent::rebuildSockets()
{
	MikanObjectPtr vrDeviceObject = getOwnerObject();
	VRDeviceComponentPtr vrDeviceComponentPtr = getSelfPtr<VRDeviceComponent>();

	// Clean up any previously created socket components
	disposeSockets();

	// Create sockets
	if (m_vrDeviceInterface != nullptr)
	{
		for (int socketIndex = 0; socketIndex < m_vrDeviceInterface->getSocketCount(); socketIndex++)
		{
			IVRDeviceSocket* vrDeviceSocket = m_vrDeviceInterface->geSocketByIndex(socketIndex);
			const std::string socketName= vrDeviceSocket->getName();

			// Create a static mesh component to hold the mesh instance
			TransformComponentPtr socketComponentPtr = vrDeviceObject->addComponent<TransformComponent>();
			socketComponentPtr->setName(socketName);
			socketComponentPtr->attachToComponent(vrDeviceComponentPtr);
			m_socketMap.insert({socketName, socketComponentPtr});
		}

		// Initialize all of the newly created sockets
		for (auto& kvpair : m_socketMap)
		{
			kvpair.second->init();
		}
	}
}

void VRDeviceComponent::getSocketNames(std::vector<std::string>& outSocketNames) const
{
	outSocketNames.clear();
	if (m_vrDeviceInterface != nullptr)
	{
		for (int socketIndex = 0; socketIndex < m_vrDeviceInterface->getSocketCount(); socketIndex++)
		{
			IVRDeviceSocket* vrDeviceSocket = m_vrDeviceInterface->geSocketByIndex(socketIndex);
			const std::string socketName = vrDeviceSocket->getName();
			outSocketNames.push_back(socketName);
		}
	}
}

bool VRDeviceComponent::getSocketRelativePoseByName(const std::string& socketName, glm::mat4& outPose) const
{
	glm::mat4 vrTrackingSpacePose = glm::mat4(1.f);

	if (m_vrDeviceInterface != nullptr)
	{
		IVRDeviceSocket* socket= m_vrDeviceInterface->getSocketByName(socketName.c_str());

		VRDevicePose socketRelativePose;
		if (socket != nullptr && socket->getSocketState(socketRelativePose))
		{
			outPose= VRDevicePose_to_GlmTransform(socketRelativePose).getMat4();

			return true;
		}
	}

	return false;
}

VRDevicePoseViewPtr VRDeviceComponent::makePoseView(
	eVRDevicePoseSpace space,
	const std::string& socketName) const
{
	return std::make_shared<VRDevicePoseView>(this, space, socketName);
}

void VRDeviceComponent::disposeMeshComponents()
{
	// Clean up any previously created mesh components
	for (auto& kvpair : m_meshComponentMap)
	{
		VRDeviceMeshInfo& meshInfo= kvpair.second;

		meshInfo.colliderComponent->dispose();
		meshInfo.triStaticMeshComponent->dispose();
		meshInfo.wireStaticMeshComponent->dispose();
	}

	// Forget about any collider components
	m_meshComponentMap.clear();
}

void VRDeviceComponent::rebuildMeshComponents()
{
	MikanObjectPtr vrDeviceObject = getOwnerObject();
	VRDeviceComponentPtr vrDeviceComponentPtr = getSelfPtr<VRDeviceComponent>();

	// Clean up any previously created mesh components
	disposeMeshComponents();

	// If a model loaded, create meshes and colliders for it
	if (m_vrDeviceInterface != nullptr)
	{
		for (int meshIndex = 0; meshIndex < m_vrDeviceInterface->getMeshCount(); meshIndex++)
		{
			IVRDeviceMesh* vrDeviceMesh= m_vrDeviceInterface->getMeshByIndex(meshIndex);
			const std::string meshName= vrDeviceMesh->getName();

			// Fetch the mesh and material resources
			IMkTriangulatedMeshConstPtr triMeshPtr = vrDeviceMesh->getTriangulatedMesh();
			IMkWireframeMeshConstPtr wireframeMeshPtr = vrDeviceMesh->getWireframeMesh();

			// Create a new static mesh instance from the mesh resources
			IMkStaticMeshInstancePtr triMeshInstancePtr =
				createMkStaticMeshInstance(
					triMeshPtr->getName(),
					triMeshPtr);
			triMeshInstancePtr->setVisible(true);
			triMeshInstancePtr->setIsVisibleToCamera("vrViewpoint", true);

			// Create a new (hidden) static mesh instance from the mesh resources
			IMkStaticMeshInstancePtr wireframeMeshInstancePtr =
				createMkStaticMeshInstance(
					"wireframe",
					wireframeMeshPtr);
			wireframeMeshInstancePtr->setVisible(false);

			// Create a static mesh component to hold the mesh instance
			StaticMeshComponentPtr triMeshComponentPtr = vrDeviceObject->addComponent<StaticMeshComponent>();
			triMeshComponentPtr->setName(triMeshPtr->getName());
			triMeshComponentPtr->setStaticMesh(triMeshInstancePtr);
			triMeshComponentPtr->attachToComponent(vrDeviceComponentPtr);

			// Create a static mesh component to hold the mesh instance
			StaticMeshComponentPtr wireMeshComponentPtr = vrDeviceObject->addComponent<StaticMeshComponent>();
			wireMeshComponentPtr->setName(wireframeMeshPtr->getName());
			wireMeshComponentPtr->setStaticMesh(wireframeMeshInstancePtr);
			wireMeshComponentPtr->attachToComponent(vrDeviceComponentPtr);

			// Add a mesh collider component that generates collision from the mesh data
			MeshColliderComponentPtr colliderPtr = vrDeviceObject->addComponent<MeshColliderComponent>();
			colliderPtr->setName(triMeshPtr->getName());
			colliderPtr->setStaticMeshComponent(triMeshComponentPtr);
			colliderPtr->attachToComponent(vrDeviceComponentPtr);

			VRDeviceMeshInfo meshInfo;
			meshInfo.triStaticMeshComponent= triMeshComponentPtr;
			meshInfo.wireStaticMeshComponent= wireMeshComponentPtr;
			meshInfo.colliderComponent= colliderPtr;

			m_meshComponentMap.insert({meshName, meshInfo});
		}

		// Update colors of all attached wireframe meshes
		updateWireframeMeshColor();

		// Initialize all of the newly created components
		for (auto& kvpair : m_meshComponentMap)
		{
			VRDeviceMeshInfo& meshInfo = kvpair.second;

			meshInfo.triStaticMeshComponent->init();
			meshInfo.wireStaticMeshComponent->init();
			meshInfo.colliderComponent->init();
		}
	}

	// Refresh the child collider list on the selection component
	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		selectionComponentPtr->rebindColliders();
	}
}

void VRDeviceComponent::refreshDevicePose()
{
	const int vrFrameDelay = 0; // Use the latest pose for rendering
	VRDevicePose vrDevicePose;
	if (m_vrDeviceInterface != nullptr &&
		m_vrDeviceInterface->getDevicePose(vrFrameDelay, vrDevicePose))
	{
		// Set the parent device transform
		setRelativeTransform(VRDevicePose_to_GlmTransform(vrDevicePose));

		// Update the child render mesh component relative transforms
		for (size_t meshIndex = 0; meshIndex < m_vrDeviceInterface->getMeshCount(); meshIndex++)
		{
			const IVRDeviceMesh* deviceMesh= m_vrDeviceInterface->getMeshByIndex(meshIndex);
			const std::string meshName = deviceMesh->getName();

			auto it = m_meshComponentMap.find(meshName);
			if (it != m_meshComponentMap.end())
			{
				const VRDeviceMeshInfo& meshInfo = it->second;

				VRDevicePose vrMeshPose;
				bool bIsVisible = false;
				if (deviceMesh->getMeshState(vrMeshPose, bIsVisible))
				{
					const GlmTransform vrMeshTransform = VRDevicePose_to_GlmTransform(vrMeshPose);

					meshInfo.triStaticMeshComponent->setRelativeTransform(vrMeshTransform);
					meshInfo.wireStaticMeshComponent->setRelativeTransform(vrMeshTransform);
					meshInfo.colliderComponent->setRelativeTransform(vrMeshTransform);

					meshInfo.triStaticMeshComponent->getStaticMesh()->setVisible(bIsVisible);
					meshInfo.wireStaticMeshComponent->getStaticMesh()->setVisible(bIsVisible);
				}
			}
		}

		// Update the child attachment component relative transforms
		for (size_t attachmentIndex = 0; attachmentIndex < m_vrDeviceInterface->getSocketCount(); attachmentIndex++)
		{
			const IVRDeviceSocket* attachment = m_vrDeviceInterface->geSocketByIndex(attachmentIndex);
			const std::string attachmentName = attachment->getName();

			auto it = m_socketMap.find(attachmentName);
			if (it != m_socketMap.end())
			{
				TransformComponentPtr attachmentComponent = it->second;

				VRDevicePose attachmentPose;
				if (attachment->getSocketState(attachmentPose))
				{
					const GlmTransform attachmentTransform = VRDevicePose_to_GlmTransform(attachmentPose);

					attachmentComponent->setRelativeTransform(attachmentTransform);
				}
			}
		}
	}
}

void VRDeviceComponent::customRender()
{
	TextStyle style = getDefaultTextStyle();

	VRDeviceDefinitionPtr anchorDefinition = getVRDeviceDefinition();
	wchar_t wszVRDeviceName[256];
	StringUtils::convertMbsToWcs(anchorDefinition->getComponentName().c_str(), wszVRDeviceName, sizeof(wszVRDeviceName));
	glm::mat4 anchorXform = getWorldTransform();
	glm::vec3 anchorPos(anchorXform[3]);

	glm::vec3 xColor = Colors::DarkRed;
	glm::vec3 yColor = Colors::DarkGreen;
	glm::vec3 zColor = Colors::DarkBlue;
	SelectionComponentPtr selectionComponent = m_selectionComponentWeakPtr.lock();
	if (selectionComponent)
	{
		if (selectionComponent->getIsSelected())
		{
			xColor = Colors::Red;
			yColor = Colors::Green;
			zColor = Colors::Blue;
		}
		else if (selectionComponent->getIsHovered())
		{
			xColor = Colors::LightGreen;
			yColor = Colors::LightGreen;
			zColor = Colors::LightBlue;
		}
	}

	drawTransformedAxes(anchorXform, 0.1f, 0.1f, 0.1f, xColor, yColor, zColor);
	drawTextAtWorldPosition(style, anchorPos, L"%s", wszVRDeviceName);
}

void VRDeviceComponent::updateWireframeMeshColor()
{
	glm::vec3 newColor = Colors::White;

	if (m_bIsSelected)
	{
		newColor = Colors::Yellow;
	}
	else if (m_bIsHovered)
	{
		newColor = Colors::LightGray;
	}
	else
	{
		newColor = Colors::DarkGray;
	}

	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		for (auto& kvpair : m_meshComponentMap)
		{
			VRDeviceMeshInfo& meshInfo = kvpair.second;
			IMkStaticMeshInstancePtr meshPtr = meshInfo.wireStaticMeshComponent->getStaticMesh();

			meshPtr->getMaterialInstance()->setVec4BySemantic(
				eUniformSemantic::diffuseColorRGBA,
				glm::vec4(newColor, 1.f));
		}
	}
}

// Selection Events
void VRDeviceComponent::onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult)
{
	m_bIsHovered = true;
	updateWireframeMeshColor();
}

void VRDeviceComponent::onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult)
{
	m_bIsHovered = false;
	updateWireframeMeshColor();
}

void VRDeviceComponent::onInteractionSelected()
{
	m_bIsSelected = true;
	updateWireframeMeshColor();
}

void VRDeviceComponent::onInteractionUnselected()
{
	m_bIsSelected = false;
	updateWireframeMeshColor();
}