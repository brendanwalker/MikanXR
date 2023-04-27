#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlStaticMeshInstance.h"
#include "GlTriangulatedMesh.h"
#include "GlWireframeMesh.h"
#include "MathTypeConversion.h"
#include "BoxColliderComponent.h"
#include "MeshColliderComponent.h"
#include "MikanObject.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "StaticMeshComponent.h"
#include "ModelStencilComponent.h"
#include "ProfileConfig.h"
#include "QuadStencilComponent.h"
#include "Renderer.h"
#include "StencilObjectSystem.h"

StencilObjectSystem* StencilObjectSystem::s_stencilObjectSystem= nullptr;

StencilObjectSystem::StencilObjectSystem() 
	: MikanObjectSystem()
{
	s_stencilObjectSystem = this;
}

StencilObjectSystem::~StencilObjectSystem()
{
	s_stencilObjectSystem = nullptr;
}

void StencilObjectSystem::init()
{
	MikanObjectSystem::init();

	StencilObjectSystemConfigConstPtr stencilSystemConfig = getStencilSystemConfigConst();

	for (QuadStencilConfigPtr quadConfig : stencilSystemConfig->quadStencilList)
	{
		createQuadStencilObject(quadConfig);
	}

	for (BoxStencilConfigPtr boxConfig : stencilSystemConfig->boxStencilList)
	{
		createBoxStencilObject(boxConfig);
	}

	for (ModelStencilConfigPtr modelConfig : stencilSystemConfig->modelStencilList)
	{
		createModelStencilObject(modelConfig);
	}
}

void StencilObjectSystem::dispose()
{
	m_quadStencilComponents.clear();
	m_boxStencilComponents.clear();
	m_modelStencilComponents.clear();
	MikanObjectSystem::dispose();
}

StencilComponentWeakPtr StencilObjectSystem::getStencilById(MikanStencilID stencilId) const
{
	QuadStencilComponentPtr quadPtr = getQuadStencilById(stencilId).lock();
	if (quadPtr)
	{
		return quadPtr;
	}

	BoxStencilComponentPtr boxPtr = getBoxStencilById(stencilId).lock();
	if (quadPtr)
	{
		return boxPtr;
	}

	ModelStencilComponentPtr modelPtr = getModelStencilById(stencilId).lock();
	if (modelPtr)
	{
		return modelPtr;
	}

	return StencilComponentWeakPtr();
}

bool StencilObjectSystem::getStencilWorldTransform(
	MikanStencilID parentStencilId, 
	glm::mat4& outXform) const
{
	StencilComponentPtr stencilComponent= getStencilById(parentStencilId).lock();
	if (stencilComponent)
	{
		outXform = stencilComponent->getStencilWorldTransform();
		return true;
	}

	return false;
}

QuadStencilComponentWeakPtr StencilObjectSystem::getQuadStencilById(MikanStencilID stencilId) const
{
	auto iter = m_quadStencilComponents.find(stencilId);
	if (iter != m_quadStencilComponents.end())
	{
		return iter->second;
	}

	return QuadStencilComponentWeakPtr();
}

QuadStencilComponentWeakPtr StencilObjectSystem::getQuadStencilByName(const std::string& stencilName) const
{
	for (auto it = m_quadStencilComponents.begin(); it != m_quadStencilComponents.end(); it++)
	{
		QuadStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getConfig()->getStencilName() == stencilName)
		{
			return componentPtr;
		}
	}

	return QuadStencilComponentWeakPtr();
}

QuadStencilComponentPtr StencilObjectSystem::addNewQuadStencil(const MikanStencilQuad& stencilInfo)
{
	StencilObjectSystemConfigPtr stencilSystemConfig = getStencilSystemConfig();

	MikanStencilID stencilId = stencilSystemConfig->addNewQuadStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		QuadStencilConfigPtr configPtr = stencilSystemConfig->getQuadStencilInfo(stencilId);
		assert(configPtr != nullptr);

		return createQuadStencilObject(configPtr);
	}

	return QuadStencilComponentPtr();
}

bool StencilObjectSystem::removeQuadStencil(MikanStencilID stencilId)
{
	getStencilSystemConfig()->removeStencil(stencilId);
	disposeQuadStencilObject(stencilId);

	return false;
}

QuadStencilComponentPtr StencilObjectSystem::createQuadStencilObject(QuadStencilConfigPtr quadConfig)
{
	MikanObjectPtr stencilObject = newObject().lock();

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Add a scene component
	SceneComponentPtr sceneComponentPtr = stencilObject->addComponent<SceneComponent>();
	stencilObject->setRootComponent(sceneComponentPtr);

	// Add a box collider
	BoxColliderComponentPtr boxColliderPtr = stencilObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(glm::vec3(quadConfig->getQuadWidth() * 0.5f, quadConfig->getQuadHeight() * 0.5f, 0.01f));

	// Add quad stencil component to the object
	QuadStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<QuadStencilComponent>();
	stencilComponentPtr->setConfig(quadConfig);
	m_quadStencilComponents.insert({quadConfig->getStencilId(), stencilComponentPtr});

	// Attach to parent anchor, if any
	MikanSpatialAnchorID parentAnchorId= quadConfig->getParentAnchorId();
	if (parentAnchorId != INVALID_MIKAN_ID)
	{
		AnchorComponentPtr parentAnchor = AnchorObjectSystem::getSystem()->getSpatialAnchorById(parentAnchorId);

		if (parentAnchor)
		{
			sceneComponentPtr->attachToComponent(parentAnchor->getOwnerObject()->getRootComponent());
		}
	}

	// Init the object once all components are added
	stencilObject->init();

	return stencilComponentPtr;
}

void StencilObjectSystem::disposeQuadStencilObject(MikanStencilID stencilId)
{
	auto it = m_quadStencilComponents.find(stencilId);
	if (it != m_quadStencilComponents.end())
	{
		QuadStencilComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_quadStencilComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());
	}
}

BoxStencilComponentWeakPtr StencilObjectSystem::getBoxStencilById(MikanStencilID stencilId) const
{
	auto iter = m_boxStencilComponents.find(stencilId);
	if (iter != m_boxStencilComponents.end())
	{
		return iter->second;
	}

	return BoxStencilComponentWeakPtr();
}

BoxStencilComponentWeakPtr StencilObjectSystem::getBoxStencilByName(const std::string& stencilName) const
{
	for (auto it = m_boxStencilComponents.begin(); it != m_boxStencilComponents.end(); it++)
	{
		BoxStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getConfig()->getStencilName() == stencilName)
		{
			return componentPtr;
		}
	}

	return BoxStencilComponentWeakPtr();
}

BoxStencilComponentPtr StencilObjectSystem::addNewBoxStencil(const MikanStencilBox& stencilInfo)
{
	StencilObjectSystemConfigPtr stencilSystemConfig = getStencilSystemConfig();

	MikanStencilID stencilId = stencilSystemConfig->addNewBoxStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		BoxStencilConfigPtr configPtr = stencilSystemConfig->getBoxStencilInfo(stencilId);
		assert(configPtr != nullptr);

		return createBoxStencilObject(configPtr);
	}

	return BoxStencilComponentPtr();
}

bool StencilObjectSystem::removeBoxStencil(MikanStencilID stencilId)
{
	getStencilSystemConfig()->removeStencil(stencilId);
	disposeBoxStencilObject(stencilId);

	return false;
}

BoxStencilComponentPtr StencilObjectSystem::createBoxStencilObject(BoxStencilConfigPtr boxConfig)
{
	MikanObjectPtr stencilObject = newObject().lock();

	// Add a scene component to the anchor
	SceneComponentPtr sceneComponentPtr = stencilObject->addComponent<SceneComponent>();
	stencilObject->setRootComponent(sceneComponentPtr);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Add a box collider
	BoxColliderComponentPtr boxColliderPtr = stencilObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(
		glm::vec3(
			boxConfig->getBoxXSize() * 0.5f, 
			boxConfig->getBoxYSize() * 0.5f, 
			boxConfig->getBoxZSize() * 0.5f));

	// Add spatial anchor component to the object
	BoxStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<BoxStencilComponent>();
	stencilComponentPtr->setConfig(boxConfig);
	m_boxStencilComponents.insert({boxConfig->getStencilId(), stencilComponentPtr});

	// Attach to parent anchor, if any
	MikanSpatialAnchorID parentAnchorId = boxConfig->getParentAnchorId();
	if (parentAnchorId != INVALID_MIKAN_ID)
	{
		AnchorComponentPtr parentAnchor = AnchorObjectSystem::getSystem()->getSpatialAnchorById(parentAnchorId);

		if (parentAnchor)
		{
			sceneComponentPtr->attachToComponent(parentAnchor->getOwnerObject()->getRootComponent());
		}
	}

	// Init the object once all components are added
	stencilObject->init();

	return stencilComponentPtr;
}

void StencilObjectSystem::disposeBoxStencilObject(MikanStencilID stencilId)
{
	auto it = m_boxStencilComponents.find(stencilId);
	if (it != m_boxStencilComponents.end())
	{
		BoxStencilComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_boxStencilComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());
	}
}

ModelStencilComponentWeakPtr StencilObjectSystem::getModelStencilById(MikanStencilID stencilId) const
{
	auto iter = m_modelStencilComponents.find(stencilId);
	if (iter != m_modelStencilComponents.end())
	{
		return iter->second;
	}

	return ModelStencilComponentWeakPtr();
}

ModelStencilComponentWeakPtr StencilObjectSystem::getModelStencilByName(const std::string& stencilName) const
{
	for (auto it = m_modelStencilComponents.begin(); it != m_modelStencilComponents.end(); it++)
	{
		ModelStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getConfig()->getStencilName() == stencilName)
		{
			return componentPtr;
		}
	}

	return ModelStencilComponentWeakPtr();
}

ModelStencilComponentPtr StencilObjectSystem::addNewModelStencil(const MikanStencilModel& stencilInfo)
{
	StencilObjectSystemConfigPtr stencilSystemConfig = getStencilSystemConfig();

	MikanStencilID stencilId = stencilSystemConfig->addNewModelStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		ModelStencilConfigPtr configPtr = stencilSystemConfig->getModelStencilConfig(stencilId);
		assert(configPtr != nullptr);

		return createModelStencilObject(configPtr);
	}

	return ModelStencilComponentPtr();
}

bool StencilObjectSystem::removeModelStencil(MikanStencilID stencilId)
{
	getStencilSystemConfig()->removeStencil(stencilId);
	disposeModelStencilObject(stencilId);

	return false;
}

ModelStencilComponentPtr StencilObjectSystem::createModelStencilObject(ModelStencilConfigPtr modelConfig)
{
	MikanObjectPtr stencilObject = newObject().lock();

	// Add a scene component to the model stencil
	SceneComponentPtr sceneComponentPtr = stencilObject->addComponent<SceneComponent>();
	stencilObject->setRootComponent(sceneComponentPtr);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Fetch the model resource
	auto& modelResourceManager = Renderer::getInstance()->getModelResourceManager();
	GlRenderModelResourcePtr modelResourcePtr = modelResourceManager->fetchRenderModel(
		modelConfig->getModelPath(),
		GlRenderModelResource::getDefaultVertexDefinition());

	// Add static tri meshes
	for (size_t meshIndex = 0; meshIndex < modelResourcePtr->getTriangulatedMeshCount(); ++meshIndex)
	{
		// Fetch the mesh and material resources
		GlTriangulatedMeshPtr triMeshPtr= modelResourcePtr->getTriangulatedMesh((int)meshIndex);
		GlMaterialInstancePtr materialInstancePtr= modelResourcePtr->getTriangulatedMeshMaterial((int)meshIndex);

		// Create a new static mesh instance from the mesh resources
		GlStaticMeshInstancePtr triMeshInstancePtr = 
			std::make_shared<GlStaticMeshInstance>(
				triMeshPtr->getName(),
				triMeshPtr,
				materialInstancePtr);

		// Create a static mesh component to hold the mesh instance
		StaticMeshComponentPtr meshComponentPtr = stencilObject->addComponent<StaticMeshComponent>();
		meshComponentPtr->setStaticMesh(triMeshInstancePtr);
		meshComponentPtr->attachToComponent(sceneComponentPtr);

		// Add a mesh collider component that generates collision from the mesh data
		MeshColliderComponentPtr colliderPtr= stencilObject->addComponent<MeshColliderComponent>();
		colliderPtr->setStaticMeshComponent(meshComponentPtr);
		meshComponentPtr->attachToComponent(sceneComponentPtr);
	}

	// Add static wireframe meshes
	for (size_t meshIndex = 0; meshIndex < modelResourcePtr->getWireframeMeshCount(); ++meshIndex)
	{
		// Fetch the mesh and material resources
		GlWireframeMeshPtr wireframeMeshPtr= modelResourcePtr->getWireframeMesh((int)meshIndex);
		GlMaterialInstancePtr materialInstancePtr= modelResourcePtr->getWireframeMeshMaterial((int)meshIndex);

		// Create a new (hidden) static mesh instance from the mesh resources
		GlStaticMeshInstancePtr wireframeMeshInstancePtr =
			std::make_shared<GlStaticMeshInstance>(
				"wireframe",
				wireframeMeshPtr,
				materialInstancePtr);
		wireframeMeshInstancePtr->setVisible(false);

		// Create a static mesh component to hold the mesh instance
		StaticMeshComponentPtr meshComponentPtr = stencilObject->addComponent<StaticMeshComponent>();
		meshComponentPtr->setStaticMesh(wireframeMeshInstancePtr);
		meshComponentPtr->attachToComponent(sceneComponentPtr);
	}

	// Add spatial anchor component to the object
	ModelStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<ModelStencilComponent>();
	stencilComponentPtr->setConfig(modelConfig);
	m_modelStencilComponents.insert({modelConfig->getStencilId(), stencilComponentPtr});

	// Attach to parent anchor, if any
	MikanSpatialAnchorID parentAnchorId = modelConfig->getParentAnchorId();
	if (parentAnchorId != INVALID_MIKAN_ID)
	{
		AnchorComponentPtr parentAnchor = AnchorObjectSystem::getSystem()->getSpatialAnchorById(parentAnchorId);

		if (parentAnchor)
		{
			sceneComponentPtr->attachToComponent(parentAnchor->getOwnerObject()->getRootComponent());
		}
	}

	// Init the object once all components are added
	stencilObject->init();

	return stencilComponentPtr;
}

void StencilObjectSystem::disposeModelStencilObject(MikanStencilID stencilId)
{
	auto it = m_modelStencilComponents.find(stencilId);
	if (it != m_modelStencilComponents.end())
	{
		ModelStencilComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_modelStencilComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());
	}
}

StencilObjectSystemConfigConstPtr StencilObjectSystem::getStencilSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->stencilConfig;
}

StencilObjectSystemConfigPtr StencilObjectSystem::getStencilSystemConfig()
{
	return std::const_pointer_cast<StencilObjectSystemConfig>(getStencilSystemConfigConst());
}