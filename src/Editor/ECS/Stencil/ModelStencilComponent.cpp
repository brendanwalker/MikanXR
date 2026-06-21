#include "AnchorObjectSystem.h"
#include "CameraObjectSystem.h"
#include "CameraComponent.h"
#include "Colors.h"
#include "ModalSelectCamera/ModalDialog_SelectCamera.h"
#include "StencilAlignment/AppStage_StencilAlignment.h"
#include "IEditorWindow.h"
#include "IMkGraphicsContext.h"
#include "IMkLineRenderer.h"
#include "MkMaterialInstance.h"
#include "MikanModelResourceManager.h"
#include "PathUtils.h"
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
#include "StencilUtils.h"
#include "ModelStencilSystem.h"
#include "MathGLM.h"
#include "MathMikan.h"
#include "MathTypeConversion.h"
#include "MeshColliderComponent.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanObject.h"
#include "MikanStencilTypes.h"
#include "AssetReferencePropertyMetaData.h"
#include "ModelAssetReference.h"
#include "ModelStencilComponent.h"
#include "MulticastDelegate.h"
#include "StringUtils.h"
#include "TextStyle.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "tinyfiledialogs.h"

// -- ModelStencilConfig -----
const std::string ModelStencilDefinition::k_modelStencilObjPathPropertyId= "model_path";

ModelStencilDefinition::ModelStencilDefinition()
	: StencilComponentDefinition()
	, m_modelAssetRefConfig(ModelAssetReferenceFactory().allocateAssetReferenceConfig())
{
}

ModelStencilDefinition::ModelStencilDefinition(MikanStencilID stencilId)
	: StencilComponentDefinition(stencilId, "", MikanTransform())
	, m_modelAssetRefConfig(ModelAssetReferenceFactory().allocateAssetReferenceConfig())
{
}

configuru::Config ModelStencilDefinition::writeToJSON()
{
	configuru::Config pt= StencilComponentDefinition::writeToJSON();

	if (m_modelAssetRefConfig)
	{
		pt[k_modelStencilObjPathPropertyId]= m_modelAssetRefConfig->writeToJSON();
	}

	return pt;
}

void ModelStencilDefinition::readFromJSON(const configuru::Config& pt)
{
	StencilComponentDefinition::readFromJSON(pt);

	if (pt.has_key(k_modelStencilObjPathPropertyId))
	{
		m_modelAssetRefConfig->readFromJSON(pt[k_modelStencilObjPathPropertyId]);
	}
}

bool ModelStencilDefinition::readFromInitParams(MikanObjectSystem* ownerObjectSystem,
												const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!StencilComponentDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* componentValues= initParams.getTypedPointer<MikanModelStencilComponentValues>();
	if (componentValues)
	{
		setModelPath(std::filesystem::path(componentValues->model_path.getValue()));
	}

	return true;
}

bool ModelStencilDefinition::hasModelPath() const { return !m_modelAssetRefConfig->assetPath.empty(); }

const std::filesystem::path ModelStencilDefinition::getModelPath() const { return m_modelAssetRefConfig->assetPath; }

void ModelStencilDefinition::setModelPath(const std::filesystem::path& path, bool bForceDirty)
{
	if (bForceDirty || path.string() != m_modelAssetRefConfig->assetPath)
	{
		m_modelAssetRefConfig->assetPath= path.string();
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_modelStencilObjPathPropertyId));
	}
}

// -- ModelStencilComponent -----
ModelStencilComponent::ModelStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
	, m_modelAssetRef(ModelAssetReferenceFactory().allocateAssetReference())
{
}

// -- IEntityAccessor ----
rfk::Struct const* ModelStencilComponent::getClientAPIValuesStructType() const
{
	return &MikanModelStencilComponentValues::staticGetArchetype();
}

void ModelStencilComponent::init()
{
	StencilComponent::init();

	ModelStencilDefinitionPtr modelStencilDefinition= getModelStencilDefinition();
	if (modelStencilDefinition)
	{
		// If the component definition has a model path, copy it to the model asset ref
		const std::filesystem::path modelPath= modelStencilDefinition->getModelPath();
		if (!modelPath.empty())
		{
			m_modelAssetRef->setAssetPath(modelPath);
		}

		// Listen for definition changes
		m_definition->OnPropertyChanged+= MakeDelegate(this, &ModelStencilComponent::onStencilDefinitionMarkedDirty);
	}

	// Create a selection component so that we can selection the mesh collision geometry
	SelectionComponentPtr selectionComponentPtr= getOwnerObject()->getComponentOfType<SelectionComponent>();
	if (selectionComponentPtr)
	{
		// Bind selection events
		selectionComponentPtr->OnInteractionRayOverlapEnter+=
			MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapEnter);
		selectionComponentPtr->OnInteractionRayOverlapExit+=
			MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapExit);
		selectionComponentPtr->OnInteractionSelected+=
			MakeDelegate(this, &ModelStencilComponent::onInteractionSelected);
		selectionComponentPtr->OnInteractionUnselected+=
			MakeDelegate(this, &ModelStencilComponent::onInteractionUnselected);
		selectionComponentPtr->OnTransformGizmoBound+=
			MakeDelegate(this, &ModelStencilComponent::onTransformGizmoBound);
		selectionComponentPtr->OnTransformGizmoUnbound+=
			MakeDelegate(this, &ModelStencilComponent::onTransformGizmoUnbound);

		// Remember the selection component
		m_selectionComponentWeakPtr= selectionComponentPtr;
	}

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
}

void ModelStencilComponent::customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const
{
	ModelStencilDefinitionPtr modelStencilDefinition= getModelStencilDefinition();
	auto editorObjectSystem= getObjectSystemOfType<EditorObjectSystem>();

	if (!modelStencilDefinition->getIsDisabled())
	{
		TextStyle style= getDefaultTextStyle();

		const glm::mat4 xform= getWorldTransform();
		const glm::vec3 position= glm::vec3(xform[3]);

		if (!m_bIsTransformGizmoBound)
		{
			drawTransformedAxes(graphicsContext, xform, 0.1f, 0.1f, 0.1f);
			drawTextAtWorldPosition(graphicsContext, style, position, L"Stencil %d",
									modelStencilDefinition->getComponentId());
		}
	}
}

void ModelStencilComponent::dispose()
{
	getModelStencilDefinition()->OnPropertyChanged-=
		MakeDelegate(this, &ModelStencilComponent::onStencilDefinitionMarkedDirty);

	SelectionComponentPtr selectionComponentPtr= m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		selectionComponentPtr->OnInteractionRayOverlapEnter-=
			MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapEnter);
		selectionComponentPtr->OnInteractionRayOverlapExit-=
			MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapExit);
		selectionComponentPtr->OnInteractionSelected-=
			MakeDelegate(this, &ModelStencilComponent::onInteractionSelected);
		selectionComponentPtr->OnInteractionUnselected-=
			MakeDelegate(this, &ModelStencilComponent::onInteractionUnselected);
		selectionComponentPtr->OnTransformGizmoBound-=
			MakeDelegate(this, &ModelStencilComponent::onTransformGizmoBound);
		selectionComponentPtr->OnTransformGizmoUnbound-=
			MakeDelegate(this, &ModelStencilComponent::onTransformGizmoUnbound);

		m_selectionComponentWeakPtr= selectionComponentPtr;
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

	SelectionComponentPtr selectionComponentPtr= m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		for (IMkStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
		{
			meshPtr->getMaterialInstance()->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA,
															  glm::vec4(newColor, 1.f));
		}
	}
}

void ModelStencilComponent::onStencilDefinitionMarkedDirty(CommonConfigPtr configPtr,
														   const ConfigPropertyChangeSet& changedPropertySet)
{
	ModelStencilDefinitionPtr modelStencilConfig= std::dynamic_pointer_cast<ModelStencilDefinition>(configPtr);

	if (modelStencilConfig != nullptr)
	{
		if (changedPropertySet.hasPropertyName(ModelStencilDefinition::k_modelStencilObjPathPropertyId))
		{
			// Copy the model path, if any, from the definition to the asset ref
			m_modelAssetRef->setAssetPath(modelStencilConfig->getModelPath());

			// (Re)Initialize the mesh components
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
		TransformComponentPtr componentPtr= m_meshComponents[m_meshComponents.size() - 1];
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
	IEditorWindow* ownerWindow= getOwnerEditorWindow();
	MikanModelResourceManager* modelResourceManager= ownerWindow->getModelResourceManager();
	MkMaterialConstPtr stencilMaterial=
		ownerWindow->getGraphicsContext()->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PNT_TEXTURED);
	MikanRenderModelResourcePtr modelResourcePtr=
		modelResourceManager->fetchRenderModel(modelStencilDefinition->getModelPath(), stencilMaterial);

	// If a model loaded, create meshes and colliders for it
	if (modelResourcePtr)
	{
		// Add static tri meshes
		for (int meshIndex= 0; meshIndex < modelResourcePtr->getTriangulatedMeshCount(); ++meshIndex)
		{
			// Fetch the mesh and material resources
			IMkTriangulatedMeshPtr triMeshPtr= modelResourcePtr->getTriangulatedMesh(meshIndex);

			// Create a new static mesh instance from the mesh resources
			IMkStaticMeshInstancePtr triMeshInstancePtr= createMkStaticMeshInstance(triMeshPtr->getName(), triMeshPtr);
			triMeshInstancePtr->setVisible(true);

			// Create a static mesh component to hold the mesh instance
			StaticMeshComponentPtr meshComponentPtr= stencilObject->addComponent<StaticMeshComponent>();
			meshComponentPtr->setName(triMeshPtr->getName());
			meshComponentPtr->setStaticMesh(triMeshInstancePtr);
			meshComponentPtr->attachToComponent(stencilComponentPtr);
			m_meshComponents.push_back(meshComponentPtr);
			m_triMeshComponents.push_back(meshComponentPtr);

			// Add a mesh collider component that generates collision from the mesh data
			MeshColliderComponentPtr colliderPtr= stencilObject->addComponent<MeshColliderComponent>();
			colliderPtr->setName(triMeshPtr->getName());
			colliderPtr->setStaticMeshComponent(meshComponentPtr);
			colliderPtr->attachToComponent(stencilComponentPtr);
			m_colliderComponents.push_back(colliderPtr);
			m_meshComponents.push_back(colliderPtr);
		}

		// Add static wireframe meshes
		for (int meshIndex= 0; meshIndex < modelResourcePtr->getWireframeMeshCount(); ++meshIndex)
		{
			// Fetch the mesh and material resources
			IMkWireframeMeshPtr wireframeMeshPtr= modelResourcePtr->getWireframeMesh(meshIndex);

			// Create a new (hidden) static mesh instance from the mesh resources
			IMkStaticMeshInstancePtr wireframeMeshInstancePtr=
				createMkStaticMeshInstance("wireframe", wireframeMeshPtr);
			m_wireframeMeshes.push_back(wireframeMeshInstancePtr);

			// Create a static mesh component to hold the mesh instance
			StaticMeshComponentPtr meshComponentPtr= stencilObject->addComponent<StaticMeshComponent>();
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

		// Initialize all of the newly created components
		for (TransformComponentPtr childComponentPtr : m_meshComponents)
		{
			childComponentPtr->postInit();
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
	m_bIsHovered= false;
	updateWireframeMeshColor();
}

void ModelStencilComponent::onInteractionSelected()
{
	m_bIsSelected= true;
	updateWireframeMeshColor();
}

void ModelStencilComponent::onInteractionUnselected()
{
	m_bIsSelected= false;
	updateWireframeMeshColor();
}

void ModelStencilComponent::onTransformGizmoBound()
{
	m_bIsTransformGizmoBound= true;
	updateWireframeMeshColor();
}

void ModelStencilComponent::onTransformGizmoUnbound()
{
	m_bIsTransformGizmoBound= false;
	updateWireframeMeshColor();
}

// -- IPropertyInterface ----
void ModelStencilComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	StencilComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
								 ModelStencilDefinition::k_modelStencilObjPathPropertyId, MikanVariantType::STRING)
								 ->addMetaData(std::make_shared<AssetReferenceFactoryMetaData>(
									 AssetReferenceFactory::createFactory<ModelAssetReferenceFactory>())));
}

bool ModelStencilComponent::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		outValue= getModelStencilDefinition()->getModelPath();
		return true;
	}

	return StencilComponent::getPropertyValue(propertyName, outValue);
}

bool ModelStencilComponent::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		const std::string fileString= inValue.getUtf8StringPointerValue();
		const std::filesystem::path filePath(fileString);

		setModelPath(filePath);
		return true;
	}

	return StencilComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
const std::string ModelStencilComponent::k_alignStencilFunctionId= "align_stencil";

void ModelStencilComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	StencilComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(std::make_shared<FunctionDescriptor>(k_alignStencilFunctionId, "Align Stencil"));
}

bool ModelStencilComponent::invokeFunction(const std::string& functionName)
{
	if (functionName == ModelStencilComponent::k_alignStencilFunctionId)
	{
		alignStencil();
	}

	return StencilComponent::invokeFunction(functionName);
}

void ModelStencilComponent::alignStencil()
{
	AppStage* ownerAppStage= getOwnerEditorWindow()->getCurrentAppStage();

	ModalDialog_SelectCamera::selectCamera(
		ownerAppStage,
		[this](MikanCameraID cameraId)
		{
			// Show Anchor Triangulation Tool
			auto* stencilAligner= getOwnerEditorWindow()->pushAppStageOfType<AppStage_StencilAlignment>();
			if (stencilAligner)
			{
				CameraComponentPtr cameraComponent=
					getObjectSystemOfType<CameraObjectSystem>()->getTypedComponentById(cameraId);

				stencilAligner->setTargetStencil(getSelfPtr<ModelStencilComponent>());
				stencilAligner->setSourceCamera(cameraComponent);
			}
		});
}

// -- Lua Binding ----
void ModelStencilComponent::bindLuaFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<ModelStencilComponent, StencilComponent>(ModelStencilComponent::k_componentClassName.c_str())
		.addProperty("modelPath", [](ModelStencilComponent* component) -> std::string
					 { return component->getModelStencilDefinition()->getModelPath().string(); })
		.addFunction("alignStencil", [](ModelStencilComponent* c) { c->alignStencil(); })
		.endClass();
}