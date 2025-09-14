#include "AnchorObjectSystem.h"
#include "CameraObjectSystem.h"
#include "Colors.h"
#include "ModalSelectCamera/ModalDialog_SelectCamera.h"
#include "StencilAlignment/AppStage_StencilAlignment.h"
#include "IMkLineRenderer.h"
#include "MkMaterialInstance.h"
#include "MikanModelResourceManager.h"
#include "MikanRenderModelResource.h"
#include "IMkTriangulatedMesh.h"
#include "IMkTextRenderer.h"
#include "MikanShaderCache.h"
#include "IMkStaticMeshInstance.h"
#include "IMkWireframeMesh.h"
#include "AnchorComponent.h"
#include "TransformComponent.h"
#include "SelectionComponent.h"
#include "StaticMeshComponent.h"
#include "StencilObjectSystem.h"
#include "StencilObjectSystemConfig.h"
#include "MathGLM.h"
#include "MathMikan.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MeshColliderComponent.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanObject.h"
#include "MikanStencilTypes.h"
#include "ModelStencilComponent.h"
#include "MulticastDelegate.h"
#include "StringUtils.h"
#include "TextStyle.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <glm/gtx/matrix_decompose.hpp>

// -- ModelStencilConfig -----
const std::string ModelStencilDefinition::k_modelStencilObjPathPropertyId = "model_path";

ModelStencilDefinition::ModelStencilDefinition()
	: StencilComponentDefinition()
{
}

ModelStencilDefinition::ModelStencilDefinition(const MikanStencilModelInfo& modelInfo)
	: StencilComponentDefinition(
		modelInfo.stencil_id, 
		modelInfo.parent_anchor_id, 
		modelInfo.stencil_name.getValue(), 
		modelInfo.relative_transform)
{
}

configuru::Config ModelStencilDefinition::writeToJSON()
{
	configuru::Config pt = StencilComponentDefinition::writeToJSON();

	pt["model_path"] = m_modelPath.string();

	return pt;
}

void ModelStencilDefinition::readFromJSON(const configuru::Config& pt)
{
	StencilComponentDefinition::readFromJSON(pt);

	m_modelPath = pt.get_or<std::string>("model_path", "");
}

void ModelStencilDefinition::setModelPath(const std::filesystem::path& path, bool bForceDirty)
{
	if (path != m_modelPath || bForceDirty)
	{
		m_modelPath = path;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilObjPathPropertyId));
	}
}

MikanStencilModelInfo ModelStencilDefinition::getModelInfo() const
{
	const std::string& modelName = getComponentName();
	GlmTransform xform = getRelativeTransform();

	MikanStencilModelInfo modelnfo = {};
	modelnfo.stencil_id = m_stencilId;
	modelnfo.parent_anchor_id = m_parentAnchorId;
	modelnfo.relative_transform = glm_transform_to_MikanTransform(getRelativeTransform());
	modelnfo.is_disabled = m_bIsDisabled;
	modelnfo.stencil_name= modelName;

	return modelnfo;
}

// -- ModelStencilComponent -----
ModelStencilComponent::ModelStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{
	m_bWantsCustomRender= true;
}

void ModelStencilComponent::init()
{
	StencilComponent::init();

	// Listen for stencil model path changes
	getModelStencilDefinition()->OnMarkedDirty+=
		MakeDelegate(this, &ModelStencilComponent::onStencilDefinitionMarkedDirty);

	// Create a selection component so that we can selection the mesh collision geometry
	SelectionComponentPtr selectionComponentPtr = getOwnerObject()->getComponentOfType<SelectionComponent>();
	if (selectionComponentPtr)
	{
		// Bind selection events
		selectionComponentPtr->OnInteractionRayOverlapEnter += MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapEnter);
		selectionComponentPtr->OnInteractionRayOverlapExit += MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapExit);
		selectionComponentPtr->OnInteractionSelected += MakeDelegate(this, &ModelStencilComponent::onInteractionSelected);
		selectionComponentPtr->OnInteractionUnselected += MakeDelegate(this, &ModelStencilComponent::onInteractionUnselected);
		selectionComponentPtr->OnTransformGizmoBound += MakeDelegate(this, &ModelStencilComponent::onTransformGizmoBound);
		selectionComponentPtr->OnTransformGizmoUnbound += MakeDelegate(this, &ModelStencilComponent::onTransformGizmoUnbound);

		// Remember the selection component
		m_selectionComponentWeakPtr = selectionComponentPtr;
	}

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
}

void ModelStencilComponent::customRender()
{
	ModelStencilDefinitionPtr modelStencilDefinition= getModelStencilDefinition();

	if (!modelStencilDefinition->getIsDisabled() &&
		StencilObjectSystem::getSystem()->getStencilSystemConfig()->getRenderStencilsFlag())
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = getWorldTransform();
		const glm::vec3 position = glm::vec3(xform[3]);

		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", modelStencilDefinition->getStencilId());
	}
}

void ModelStencilComponent::dispose()
{
	getModelStencilDefinition()->OnMarkedDirty -=
		MakeDelegate(this, &ModelStencilComponent::onStencilDefinitionMarkedDirty);

	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		selectionComponentPtr->OnInteractionRayOverlapEnter -= MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapEnter);
		selectionComponentPtr->OnInteractionRayOverlapExit -= MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapExit);
		selectionComponentPtr->OnInteractionSelected -= MakeDelegate(this, &ModelStencilComponent::onInteractionSelected);
		selectionComponentPtr->OnInteractionUnselected -= MakeDelegate(this, &ModelStencilComponent::onInteractionUnselected);
		selectionComponentPtr->OnTransformGizmoBound -= MakeDelegate(this, &ModelStencilComponent::onTransformGizmoBound);
		selectionComponentPtr->OnTransformGizmoUnbound -= MakeDelegate(this, &ModelStencilComponent::onTransformGizmoUnbound);

		m_selectionComponentWeakPtr = selectionComponentPtr;
	}

	StencilComponent::dispose();
}

void ModelStencilComponent::setRenderStencilsFlag(bool flag)
{
	for (IMkStaticMeshInstancePtr mesh : m_wireframeMeshes)
	{
		mesh->setVisible(flag);
	}
}

void ModelStencilComponent::updateWireframeMeshColor()
{
	glm::vec3 newColor= Colors::White;

	if (m_bIsTransformGizmoBound)
	{
		newColor= Colors::GreenYellow;
	}
	else if (m_bIsSelected)
	{
		newColor= Colors::Yellow;
	}
	else if (m_bIsHovered)
	{
		newColor= Colors::LightGray;
	}
	else
	{
		newColor= Colors::DarkGray;
	}

	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		for (IMkStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
		{
			meshPtr->getMaterialInstance()->setVec4BySemantic(
				eUniformSemantic::diffuseColorRGBA,
				glm::vec4(newColor, 1.f));
		}
	}
}

void ModelStencilComponent::onStencilDefinitionMarkedDirty(
	CommonConfigPtr configPtr, 
	const ConfigPropertyChangeSet& changedPropertySet)
{
	ModelStencilDefinitionPtr modelStencilConfig = std::dynamic_pointer_cast<ModelStencilDefinition>(configPtr);

	if (modelStencilConfig != nullptr)
	{
		if (changedPropertySet.hasPropertyName(ModelStencilDefinition::k_modelStencilObjPathPropertyId))
		{
			rebuildMeshComponents();
		}
	}
}

void ModelStencilComponent::setModelPath(const std::filesystem::path& path)
{
	ModelStencilDefinitionPtr modelStencilDefinition= getModelStencilDefinition();

	if (path == modelStencilDefinition->getModelPath())
		return;

	// This fires off a config change event, which causes rebuildMeshComponents to be called
	modelStencilDefinition->setModelPath(path);
}

void ModelStencilComponent::disposeMeshComponents()
{
	// Clean up any previously created mesh components
	while (m_meshComponents.size() > 0)
	{
		TransformComponentPtr componentPtr = m_meshComponents[m_meshComponents.size() - 1];
		componentPtr->dispose();

		m_meshComponents.pop_back();
	}

	// Forget about any collider components
	m_colliderComponents.clear();

	// Forget about the triangulated meshes
	m_triMeshComponents.clear();

	// Forget about any wireframe meshes
	m_wireframeMeshes.clear();
}

void ModelStencilComponent::rebuildMeshComponents()
{
	ModelStencilDefinitionPtr modelStencilDefinition= getModelStencilDefinition();
	MikanObjectPtr stencilObject= getOwnerObject();
	StencilComponentPtr stencilComponentPtr= getSelfPtr<StencilComponent>();

	// Clean up any previously created mesh components
	disposeMeshComponents();

	// Fetch the stencil model resource
	// TODO: Need to consider how MikanObjects are rendered across multiple windows,
	// since each window needs to own its own models and shader resources.
	// For now, we are assuming that models are only rendered in the Main Window.
	MainWindow* mainWindow= MainWindow::getInstance();
	MikanModelResourceManager* modelResourceManager= mainWindow->getModelResourceManager();
	MkMaterialConstPtr stencilMaterial= 
		mainWindow->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PNT_TEXTURED);
	MikanRenderModelResourcePtr modelResourcePtr= 
		modelResourceManager->fetchRenderModel(
			modelStencilDefinition->getModelPath(), stencilMaterial);

	// If a model loaded, create meshes and colliders for it
	if (modelResourcePtr)
	{
		// Add static tri meshes
		for (int meshIndex = 0; meshIndex < modelResourcePtr->getTriangulatedMeshCount(); ++meshIndex)
		{
			// Fetch the mesh and material resources
			IMkTriangulatedMeshPtr triMeshPtr = modelResourcePtr->getTriangulatedMesh(meshIndex);

			// Create a new static mesh instance from the mesh resources
			IMkStaticMeshInstancePtr triMeshInstancePtr =
				createMkStaticMeshInstance(
					triMeshPtr->getName(),
					triMeshPtr);
			triMeshInstancePtr->setVisible(true);
			triMeshInstancePtr->setIsVisibleToCamera("vrViewpoint", true);

			// Create a static mesh component to hold the mesh instance
			StaticMeshComponentPtr meshComponentPtr = stencilObject->addComponent<StaticMeshComponent>();
			meshComponentPtr->setName(triMeshPtr->getName());
			meshComponentPtr->setStaticMesh(triMeshInstancePtr);
			meshComponentPtr->attachToComponent(stencilComponentPtr);
			m_meshComponents.push_back(meshComponentPtr);
			m_triMeshComponents.push_back(meshComponentPtr);

			// Add a mesh collider component that generates collision from the mesh data
			MeshColliderComponentPtr colliderPtr = stencilObject->addComponent<MeshColliderComponent>();
			colliderPtr->setName(triMeshPtr->getName());
			colliderPtr->setStaticMeshComponent(meshComponentPtr);
			colliderPtr->attachToComponent(stencilComponentPtr);
			m_colliderComponents.push_back(colliderPtr);
			m_meshComponents.push_back(colliderPtr);
		}

		// Add static wireframe meshes
		for (int meshIndex = 0; meshIndex < modelResourcePtr->getWireframeMeshCount(); ++meshIndex)
		{
			// Fetch the mesh and material resources
			IMkWireframeMeshPtr wireframeMeshPtr = modelResourcePtr->getWireframeMesh(meshIndex);

			// Create a new (hidden) static mesh instance from the mesh resources
			IMkStaticMeshInstancePtr wireframeMeshInstancePtr =
				createMkStaticMeshInstance(
					"wireframe",
					wireframeMeshPtr);
			m_wireframeMeshes.push_back(wireframeMeshInstancePtr);

			// Create a static mesh component to hold the mesh instance
			StaticMeshComponentPtr meshComponentPtr = stencilObject->addComponent<StaticMeshComponent>();
			meshComponentPtr->setName(wireframeMeshPtr->getName());
			meshComponentPtr->setStaticMesh(wireframeMeshInstancePtr);
			meshComponentPtr->attachToComponent(stencilComponentPtr);
			m_meshComponents.push_back(meshComponentPtr);
		}

		// Update colors of all attached wireframe meshes
		updateWireframeMeshColor();

		// Initialize all of the newly created components
		for (TransformComponentPtr childComponentPtr : m_meshComponents)
		{
			childComponentPtr->init();
		}
	}

	// Refresh the child collider list on the selection component
	SelectionComponentPtr selectionComponentPtr= m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		selectionComponentPtr->rebindColliders();
	}
}

void ModelStencilComponent::extractRenderGeometry(MikanStencilModelRenderGeometry& outRenderGeometry)
{
	for (StaticMeshComponentPtr mesh : m_triMeshComponents)
	{
		MikanTriagulatedMesh mikanMesh= {};
		mesh->extractRenderGeometry(mikanMesh);

		outRenderGeometry.meshes.push_back(mikanMesh);
	}
}

void ModelStencilComponent::onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult)
{
	m_bIsHovered= true;
	updateWireframeMeshColor();
}

void ModelStencilComponent::onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult)
{
	m_bIsHovered = false;
	updateWireframeMeshColor();
}

void ModelStencilComponent::onInteractionSelected()
{
	m_bIsSelected = true;
	updateWireframeMeshColor();
}

void ModelStencilComponent::onInteractionUnselected()
{
	m_bIsSelected = false;
	updateWireframeMeshColor();
}

void ModelStencilComponent::onTransformGizmoBound()
{
	m_bIsTransformGizmoBound= true;
	updateWireframeMeshColor();
}

void ModelStencilComponent::onTransformGizmoUnbound()
{
	m_bIsTransformGizmoBound = false;
	updateWireframeMeshColor();
}

// -- IRmlPropertyInterface ----
void ModelStencilComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	StencilComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			ModelStencilDefinition::k_modelStencilObjPathPropertyId));
}

bool ModelStencilComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		Rml::String filepath = getModelStencilDefinition()->getModelPath().string();

		outValue = filepath;
		return true;
	}

	return StencilComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool ModelStencilComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		const Rml::String fileString = inValue.Get<Rml::String>();
		const std::filesystem::path filePath(fileString);

		setModelPath(filePath);
		return true;
	}

	return StencilComponent::setPropertyValueFromRml(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
const std::string ModelStencilComponent::k_alignStencilFunctionId = "align_stencil";

void ModelStencilComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	StencilComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_alignStencilFunctionId, "Align Stencil"));
}

bool ModelStencilComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionName = functionDesc->getFunctionName();

	if (functionName == ModelStencilComponent::k_alignStencilFunctionId)
	{
		alignStencil();
	}

	return StencilComponent::invokeFunctionFromRml(functionDesc);
}

void ModelStencilComponent::alignStencil()
{
	ModalDialog_SelectCamera::selectCamera(
		[this](MikanCameraID cameraId) {
			// Show Anchor Triangulation Tool
			auto* stencilAligner = MainWindow::getInstance()->pushAppStageOfType<AppStage_StencilAlignment>();
			if (stencilAligner)
			{
				stencilAligner->setTargetStencil(getSelfPtr<ModelStencilComponent>());
				stencilAligner->setSourceCamera(CameraObjectSystem::getSystem()->getCameraById(cameraId));
			}
		});
}