#include "AnchorObjectSystem.h"
#include "Colors.h"
#include "DepthMeshCapture/AppStage_DepthMeshCapture.h"
#include "GlLineRenderer.h"
#include "GlMaterialInstance.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlTriangulatedMesh.h"
#include "GlTextRenderer.h"
#include "GlShaderCache.h"
#include "GlStaticMeshInstance.h"
#include "GlWireframeMesh.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "StaticMeshComponent.h"
#include "StencilObjectSystem.h"
#include "StencilObjectSystemConfig.h"
#include "MathGLM.h"
#include "MathMikan.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MeshColliderComponent.h"
#include "MikanObject.h"
#include "ModelStencilComponent.h"
#include "MulticastDelegate.h"
#include "StringUtils.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <glm/gtx/matrix_decompose.hpp>

// -- ModelStencilConfig -----
const std::string ModelStencilDefinition::k_modelStencilObjPathPropertyId = "model_path";
const std::string ModelStencilDefinition::k_modelStencilIsDepthMeshPropertyId = "is_depth_mesh";

ModelStencilDefinition::ModelStencilDefinition()
	: StencilComponentDefinition()
{
}

ModelStencilDefinition::ModelStencilDefinition(const MikanStencilModel& modelInfo)
	: StencilComponentDefinition(
		modelInfo.stencil_id, 
		modelInfo.parent_anchor_id, 
		modelInfo.stencil_name, 
		modelInfo.relative_transform)
{
}

configuru::Config ModelStencilDefinition::writeToJSON()
{
	configuru::Config pt = StencilComponentDefinition::writeToJSON();

	pt["model_path"] = m_modelPath.string();
	pt["is_depth_mesh"] = m_bIsDepthMesh;

	return pt;
}

void ModelStencilDefinition::readFromJSON(const configuru::Config& pt)
{
	StencilComponentDefinition::readFromJSON(pt);

	m_modelPath = pt.get_or<std::string>("model_path", "");
	m_bIsDepthMesh = pt.get_or<bool>("is_depth_mesh", false);
}

void ModelStencilDefinition::setModelPath(const std::filesystem::path& path, bool bForceDirty)
{
	if (path != m_modelPath || bForceDirty)
	{
		m_modelPath = path;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilObjPathPropertyId));
	}
}

void ModelStencilDefinition::setIsDepthMesh(bool isDepthMesh)
{
	if (m_bIsDepthMesh != isDepthMesh)
	{
		m_bIsDepthMesh = isDepthMesh;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilIsDepthMeshPropertyId));
	}
}

bool ModelStencilDefinition::hasValidDepthMesh() const
{
	if (m_bIsDepthMesh && !m_modelPath.empty() && std::filesystem::exists(m_modelPath))
	{
		return true;
	}

	return false;
}

MikanStencilModel ModelStencilDefinition::getModelInfo() const
{
	const std::string& modelName = getComponentName();
	GlmTransform xform = getRelativeTransform();

	MikanStencilModel modelnfo = {};
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
	for (GlStaticMeshInstancePtr mesh : m_wireframeMeshes)
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
		for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
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
		SceneComponentPtr componentPtr = m_meshComponents[m_meshComponents.size() - 1];
		componentPtr->dispose();

		m_meshComponents.pop_back();
	}

	// Forget about any collider components
	m_colliderComponents.clear();

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
	GlModelResourceManager* modelResourceManager= mainWindow->getModelResourceManager();
	GlMaterialConstPtr stencilMaterial= 
		mainWindow->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PT_TEXTURED);
	GlRenderModelResourcePtr modelResourcePtr= 
		modelResourceManager->fetchRenderModel(
			modelStencilDefinition->getModelPath(), stencilMaterial);

	// If a model loaded, create meshes and colliders for it
	if (modelResourcePtr)
	{
		// Add static tri meshes
		for (int meshIndex = 0; meshIndex < modelResourcePtr->getTriangulatedMeshCount(); ++meshIndex)
		{
			// Fetch the mesh and material resources
			GlTriangulatedMeshPtr triMeshPtr = modelResourcePtr->getTriangulatedMesh(meshIndex);

			// Create a new static mesh instance from the mesh resources
			GlStaticMeshInstancePtr triMeshInstancePtr =
				std::make_shared<GlStaticMeshInstance>(
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
			GlWireframeMeshPtr wireframeMeshPtr = modelResourcePtr->getWireframeMesh(meshIndex);

			// Create a new (hidden) static mesh instance from the mesh resources
			GlStaticMeshInstancePtr wireframeMeshInstancePtr =
				std::make_shared<GlStaticMeshInstance>(
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
		for (SceneComponentPtr childComponentPtr : m_meshComponents)
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

// -- IPropertyInterface ----
void ModelStencilComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	StencilComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(ModelStencilDefinition::k_modelStencilObjPathPropertyId);
}

bool ModelStencilComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (StencilComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		outDescriptor = {ModelStencilDefinition::k_modelStencilObjPathPropertyId, ePropertyDataType::datatype_string, ePropertySemantic::filename};
		return true;
	}

	return false;
}

bool ModelStencilComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (StencilComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		Rml::String filepath= getModelStencilDefinition()->getModelPath().string();

		outValue= filepath;
		return true;
	}

	return false;
}

bool ModelStencilComponent::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{
	if (StencilComponent::getPropertyAttribute(propertyName, attributeName, outValue))
		return true;

	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		if (attributeName == *k_PropertyAttributeFileBrowseTitle)
		{
			outValue= "Select a model";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilter)
		{
			outValue = ".obj";
		}
	}

	return false;
}

bool ModelStencilComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (StencilComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		const Rml::String fileString= inValue.Get<Rml::String>();
		const std::filesystem::path filePath(fileString);

		setModelPath(filePath);
		return true;
	}

	return false;
}
// -- IFunctionInterface ----
const std::string ModelStencilComponent::k_createDepthMeshFunctionId = "create_depth_mesh";

void ModelStencilComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	StencilComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_createDepthMeshFunctionId);
}

bool ModelStencilComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (StencilComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == ModelStencilComponent::k_createDepthMeshFunctionId)
	{
		outDescriptor = {ModelStencilComponent::k_createDepthMeshFunctionId, "Make Depth Mesh"};
		return true;
	}

	return false;
}

bool ModelStencilComponent::invokeFunction(const std::string& functionName)
{
	if (StencilComponent::invokeFunction(functionName))
		return true;

	if (functionName == ModelStencilComponent::k_createDepthMeshFunctionId)
	{
		createDepthMesh();
	}

	return false;
}

void ModelStencilComponent::createDepthMesh()
{
	// Show Anchor Triangulation Tool
	auto* depthMeshCapture = MainWindow::getInstance()->pushAppStage<AppStage_DepthMeshCapture>();
	if (depthMeshCapture)
	{
		ModelStencilDefinitionPtr definition = getModelStencilDefinition();

		depthMeshCapture->setTargetModelStencil(definition);
	}
}