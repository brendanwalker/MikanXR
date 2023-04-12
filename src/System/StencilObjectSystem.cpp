#include "App.h"
#include "BoxStencilComponent.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlStaticMeshInstance.h"
#include "GlTriangulatedMesh.h"
#include "GlWireframeMesh.h"
#include "MathTypeConversion.h"
#include "BoxColliderComponent.h"
#include "MeshColliderComponent.h"
#include "MikanObject.h""
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

	const StencilObjectSystemConfig& stencilConfig = getStencilConfigConst();

	for (const MikanStencilQuad& quadInfo : stencilConfig.quadStencilList)
	{
		createQuadStencilObject(quadInfo);
	}

	for (const MikanStencilBox& boxInfo : stencilConfig.boxStencilList)
	{
		createBoxStencilObject(boxInfo);
	}

	for (const MikanStencilModelConfig& modelInfo : stencilConfig.modelStencilList)
	{
		createModelStencilObject(modelInfo);
	}
}

void StencilObjectSystem::dispose()
{
	m_quadStencilComponents.clear();
	m_boxStencilComponents.clear();
	m_modelStencilComponents.clear();
	MikanObjectSystem::dispose();
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

		if (componentPtr && componentPtr->getStencilName() == stencilName)
		{
			return componentPtr;
		}
	}

	return QuadStencilComponentWeakPtr();
}

QuadStencilComponentPtr StencilObjectSystem::addNewQuadStencil(const MikanStencilQuad& stencilInfo)
{
	StencilObjectSystemConfig& anchorConfig = getStencilConfig();

	MikanStencilID stencilId = anchorConfig.addNewQuadStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		const MikanStencilQuad* stencilInfo = anchorConfig.getQuadStencilInfo(stencilId);
		assert(stencilInfo != nullptr);
	}

	return QuadStencilComponentPtr();
}

bool StencilObjectSystem::removeQuadStencil(MikanStencilID stencilId)
{
	getStencilConfig().removeStencil(stencilId);
	disposeQuadStencilObject(stencilId);

	return false;
}

QuadStencilComponentPtr StencilObjectSystem::createQuadStencilObject(const MikanStencilQuad& stencilInfo)
{
	MikanObjectPtr stencilObject = newObject();

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Add a scene component
	SceneComponentPtr sceneComponentPtr = stencilObject->addComponent<SceneComponent>();
	stencilObject->setRootComponent(sceneComponentPtr);

	// Add a box collider
	BoxColliderComponentPtr boxColliderPtr = stencilObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(glm::vec3(stencilInfo.quad_width * 0.5f, stencilInfo.quad_height * 0.5f, 0.01f));

	// Add quad stencil component to the object
	QuadStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<QuadStencilComponent>();
	stencilComponentPtr->setQuadStencil(stencilInfo);
	m_quadStencilComponents.insert({stencilInfo.stencil_id, stencilComponentPtr});

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
		removeObject(stencilComponentPtr->getOwnerObject());
	}
}

BoxStencilComponentPtr StencilObjectSystem::createBoxStencilObject(const MikanStencilBox& stencilInfo)
{
	MikanObjectPtr stencilObject = newObject();

	// Add a scene component to the anchor
	SceneComponentPtr sceneComponentPtr = stencilObject->addComponent<SceneComponent>();
	stencilObject->setRootComponent(sceneComponentPtr);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Add a box collider
	BoxColliderComponentPtr boxColliderPtr = stencilObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(
		glm::vec3(
			stencilInfo.box_x_size * 0.5f, 
			stencilInfo.box_y_size * 0.5f, 
			stencilInfo.box_z_size * 0.5f));

	// Add spatial anchor component to the object
	BoxStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<BoxStencilComponent>();
	stencilComponentPtr->setBoxStencil(stencilInfo);
	m_boxStencilComponents.insert({stencilInfo.stencil_id, stencilComponentPtr});

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
		removeObject(stencilComponentPtr->getOwnerObject());
	}
}

ModelStencilComponentPtr StencilObjectSystem::createModelStencilObject(const MikanStencilModelConfig& modelInfo)
{
	MikanObjectPtr stencilObject = newObject();

	// Add a scene component to the model stencil
	SceneComponentPtr sceneComponentPtr = stencilObject->addComponent<SceneComponent>();
	stencilObject->setRootComponent(sceneComponentPtr);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Fetch the model resource
	auto& modelResourceManager = Renderer::getInstance()->getModelResourceManager();
	GlRenderModelResourcePtr modelResourcePtr = modelResourceManager->fetchRenderModel(
		modelInfo.modelPath,
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
	stencilComponentPtr->setModelStencil(modelInfo.modelInfo);
	m_modelStencilComponents.insert({modelInfo.modelInfo.stencil_id, stencilComponentPtr});

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
		removeObject(stencilComponentPtr->getOwnerObject());
	}
}

const StencilObjectSystemConfig& StencilObjectSystem::getStencilConfigConst() const
{
	return App::getInstance()->getProfileConfig()->stencilConfig;
}

StencilObjectSystemConfig& StencilObjectSystem::getStencilConfig()
{
	return const_cast<StencilObjectSystemConfig&>(getStencilConfigConst());
}