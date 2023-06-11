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

StencilObjectSystemWeakPtr StencilObjectSystem::s_stencilObjectSystem;

bool StencilObjectSystem::init()
{
	MikanObjectSystem::init();

	StencilObjectSystemConfigConstPtr stencilSystemConfig = getStencilSystemConfigConst();

	for (QuadStencilDefinitionPtr quadConfig : stencilSystemConfig->quadStencilList)
	{
		createQuadStencilObject(quadConfig);
	}

	for (BoxStencilDefinitionPtr boxConfig : stencilSystemConfig->boxStencilList)
	{
		createBoxStencilObject(boxConfig);
	}

	for (ModelStencilDefinitionPtr modelConfig : stencilSystemConfig->modelStencilList)
	{
		createModelStencilObject(modelConfig);
	}

	s_stencilObjectSystem = std::static_pointer_cast<StencilObjectSystem>(shared_from_this());
	return true;
}

void StencilObjectSystem::dispose()
{
	s_stencilObjectSystem.reset();
	m_quadStencilComponents.clear();
	m_boxStencilComponents.clear();
	m_modelStencilComponents.clear();

	MikanObjectSystem::dispose();
}

void StencilObjectSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	QuadStencilComponentPtr quadStencil= objectPtr->getComponentOfType<QuadStencilComponent>();
	if (quadStencil != nullptr)
	{
		removeQuadStencil(quadStencil->getStencilComponentDefinition()->getStencilId());
		return;
	}

	BoxStencilComponentPtr boxStencil = objectPtr->getComponentOfType<BoxStencilComponent>();
	if (boxStencil != nullptr)
	{
		removeBoxStencil(boxStencil->getStencilComponentDefinition()->getStencilId());
		return;
	}

	ModelStencilComponentPtr modelStencil = objectPtr->getComponentOfType<ModelStencilComponent>();
	if (modelStencil != nullptr)
	{
		removeModelStencil(modelStencil->getStencilComponentDefinition()->getStencilId());
		return;
	}
}

StencilComponentPtr StencilObjectSystem::getStencilById(MikanStencilID stencilId) const
{
	QuadStencilComponentPtr quadPtr = getQuadStencilById(stencilId);
	if (quadPtr)
	{
		return quadPtr;
	}

	BoxStencilComponentPtr boxPtr = getBoxStencilById(stencilId);
	if (boxPtr)
	{
		return boxPtr;
	}

	ModelStencilComponentPtr modelPtr = getModelStencilById(stencilId);
	if (modelPtr)
	{
		return modelPtr;
	}

	return StencilComponentPtr();
}

eStencilType StencilObjectSystem::getStencilType(MikanStencilID stencilId) const
{
	QuadStencilComponentPtr quadPtr = getQuadStencilById(stencilId);
	if (quadPtr)
	{
		return eStencilType::quad;
	}

	BoxStencilComponentPtr boxPtr = getBoxStencilById(stencilId);
	if (boxPtr)
	{
		return eStencilType::box;
	}

	ModelStencilComponentPtr modelPtr = getModelStencilById(stencilId);
	if (modelPtr)
	{
		return eStencilType::model;
	}

	return eStencilType::INVALID;
}

bool StencilObjectSystem::getStencilWorldTransform(
	MikanStencilID parentStencilId, 
	glm::mat4& outXform) const
{
	StencilComponentPtr stencilComponent= getStencilById(parentStencilId);
	if (stencilComponent)
	{
		outXform = stencilComponent->getWorldTransform();
		return true;
	}

	return false;
}

bool StencilObjectSystem::removeStencil(MikanStencilID stencilId)
{
	switch (getStencilType(stencilId))
	{
	case eStencilType::quad:
		return removeQuadStencil(stencilId);
		break;
	case eStencilType::box:
		return removeBoxStencil(stencilId);
		break;
	case eStencilType::model:
		return removeModelStencil(stencilId);
		break;
	}

	return false;
}

void StencilObjectSystem::setRenderStencilsFlag(bool flag)
{
	// Update visibility on all model stencils
	for (auto it = m_modelStencilComponents.begin(); it != m_modelStencilComponents.end(); it++)
	{
		ModelStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr)
		{
			componentPtr->setRenderStencilsFlag(flag);
		}
	}

	// Forward flag on to the config
	getStencilSystemConfig()->setRenderStencilsFlag(flag);
}

QuadStencilComponentPtr StencilObjectSystem::getQuadStencilById(MikanStencilID stencilId) const
{
	auto iter = m_quadStencilComponents.find(stencilId);
	if (iter != m_quadStencilComponents.end())
	{
		return iter->second.lock();
	}

	return QuadStencilComponentPtr();
}

QuadStencilComponentPtr StencilObjectSystem::getQuadStencilByName(const std::string& stencilName) const
{
	for (auto it = m_quadStencilComponents.begin(); it != m_quadStencilComponents.end(); it++)
	{
		QuadStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == stencilName)
		{
			return componentPtr;
		}
	}

	return QuadStencilComponentPtr();
}

QuadStencilComponentPtr StencilObjectSystem::addNewQuadStencil(const MikanStencilQuad& stencilInfo)
{
	StencilObjectSystemConfigPtr stencilSystemConfig = getStencilSystemConfig();

	MikanStencilID stencilId = stencilSystemConfig->addNewQuadStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		QuadStencilDefinitionPtr configPtr = stencilSystemConfig->getQuadStencilConfig(stencilId);
		assert(configPtr != nullptr);

		return createQuadStencilObject(configPtr);
	}

	return QuadStencilComponentPtr();
}

bool StencilObjectSystem::removeQuadStencil(MikanStencilID stencilId)
{
	return
		disposeQuadStencilObject(stencilId) &&
		getStencilSystemConfig()->removeStencil(stencilId);
}

void StencilObjectSystem::getRelevantQuadStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<QuadStencilComponentPtr>& outStencilList) const
{
	outStencilList.clear();
	for (auto it = m_quadStencilComponents.begin(); it != m_quadStencilComponents.end(); it++)
	{
		MikanStencilID stencilId= it->first;
		QuadStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr->getStencilComponentDefinition()->getIsDisabled())
			continue;

		// If there is an active allow list, make sure stencil is on it
		if (allowedStencilIds != nullptr)
		{
			if (std::find(allowedStencilIds->begin(), allowedStencilIds->end(), stencilId) == allowedStencilIds->end())
			{
				continue;
			}
		}

		{
			const glm::mat4 worldXform = componentPtr->getWorldTransform();
			const glm::vec3 stencilCenter = glm::vec3(worldXform[3]); // position is 3rd column
			const glm::vec3 stencilForward = glm::vec3(worldXform[2]); // forward is 2nd column
			const glm::vec3 cameraToStencil = stencilCenter - cameraPosition;

			// Stencil is in front of the camera
			// Stencil is facing the camera (or double sided)
			if (glm::dot(cameraToStencil, cameraForward) > 0.f &&
				(componentPtr->getQuadStencilDefinition()->getIsDoubleSided() || glm::dot(stencilForward, cameraForward) < 0.f))
			{
				outStencilList.push_back(componentPtr);
			}
		}
	}
}

QuadStencilComponentPtr StencilObjectSystem::createQuadStencilObject(QuadStencilDefinitionPtr quadConfig)
{
	MikanObjectPtr stencilObject = newObject();
	stencilObject->setName(quadConfig->getComponentName());

	// Make the QuadStencil component the root of the object
	QuadStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<QuadStencilComponent>();
	stencilObject->setRootComponent(stencilComponentPtr);
	stencilComponentPtr->setDefinition(quadConfig);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Attach a box collider to quad stencil component
	BoxColliderComponentPtr boxColliderPtr = stencilObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(glm::vec3(quadConfig->getQuadWidth() * 0.5f, quadConfig->getQuadHeight() * 0.5f, 0.01f));
	boxColliderPtr->attachToComponent(stencilComponentPtr);

	// Init the object once all components are added
	stencilObject->init();

	// Keep track of all the quad stencils in the stencil system
	m_quadStencilComponents.insert({quadConfig->getStencilId(), stencilComponentPtr});

	return stencilComponentPtr;
}

bool StencilObjectSystem::disposeQuadStencilObject(MikanStencilID stencilId)
{
	auto it = m_quadStencilComponents.find(stencilId);
	if (it != m_quadStencilComponents.end())
	{
		QuadStencilComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_quadStencilComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}

BoxStencilComponentPtr StencilObjectSystem::getBoxStencilById(MikanStencilID stencilId) const
{
	auto iter = m_boxStencilComponents.find(stencilId);
	if (iter != m_boxStencilComponents.end())
	{
		return iter->second.lock();
	}

	return BoxStencilComponentPtr();
}

BoxStencilComponentPtr StencilObjectSystem::getBoxStencilByName(const std::string& stencilName) const
{
	for (auto it = m_boxStencilComponents.begin(); it != m_boxStencilComponents.end(); it++)
	{
		BoxStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == stencilName)
		{
			return componentPtr;
		}
	}

	return BoxStencilComponentPtr();
}

BoxStencilComponentPtr StencilObjectSystem::addNewBoxStencil(const MikanStencilBox& stencilInfo)
{
	StencilObjectSystemConfigPtr stencilSystemConfig = getStencilSystemConfig();

	MikanStencilID stencilId = stencilSystemConfig->addNewBoxStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		BoxStencilDefinitionPtr configPtr = stencilSystemConfig->getBoxStencilConfig(stencilId);
		assert(configPtr != nullptr);

		return createBoxStencilObject(configPtr);
	}

	return BoxStencilComponentPtr();
}

bool StencilObjectSystem::removeBoxStencil(MikanStencilID stencilId)
{
	return
		disposeBoxStencilObject(stencilId) &&
		getStencilSystemConfig()->removeStencil(stencilId);
}

void StencilObjectSystem::getRelevantBoxStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<BoxStencilComponentPtr>& outStencilList) const
{
	outStencilList.clear();
	for (auto it = m_boxStencilComponents.begin(); it != m_boxStencilComponents.end(); it++)
	{
		MikanSpatialAnchorID stencilId= it->first;
		BoxStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr->getStencilComponentDefinition()->getIsDisabled())
			continue;

		// If there is an active allow list, make sure stencil is on it
		if (allowedStencilIds != nullptr)
		{
			if (std::find(
				allowedStencilIds->begin(), allowedStencilIds->end(), stencilId)
				== allowedStencilIds->end())
			{
				continue;
			}
		}

		{
			const glm::mat4 worldXform = componentPtr->getWorldTransform();
			const glm::vec3 stencilCenter = glm::vec3(worldXform[3]); // position is 3rd column
			const glm::vec3 stencilZAxis = glm::vec3(worldXform[2]); // Z is 2nd column
			const glm::vec3 stencilYAxis = glm::vec3(worldXform[1]); // Y is 1st column
			const glm::vec3 stencilXAxis = glm::vec3(worldXform[0]); // X is 0th column
			BoxStencilDefinitionConstPtr configPtr= componentPtr->getBoxStencilDefinition();
			const float boxXSize= configPtr->getBoxXSize();
			const float boxYSize= configPtr->getBoxYSize();
			const float boxZSize= configPtr->getBoxZSize();
			const glm::vec3 cameraToStencil = stencilCenter - cameraPosition;

			const bool bIsStencilInFrontOfCamera = glm::dot(cameraToStencil, cameraForward) > 0.f;
			const bool bIsCameraInStecil =
				fabsf(glm::dot(cameraToStencil, stencilXAxis)) <= boxXSize &&
				fabsf(glm::dot(cameraToStencil, stencilYAxis)) <= boxYSize &&
				fabsf(glm::dot(cameraToStencil, stencilZAxis)) <= boxZSize;

			if (bIsStencilInFrontOfCamera || bIsCameraInStecil)
			{
				outStencilList.push_back(componentPtr);
			}
		}
	}
}

BoxStencilComponentPtr StencilObjectSystem::createBoxStencilObject(BoxStencilDefinitionPtr boxConfig)
{
	MikanObjectPtr stencilObject = newObject();
	stencilObject->setName(boxConfig->getComponentName());

	// Make the box stencil the root scene component
	BoxStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<BoxStencilComponent>();
	stencilObject->setRootComponent(stencilComponentPtr);
	stencilComponentPtr->setDefinition(boxConfig);

	// Attach a box collider component to the stencil
	BoxColliderComponentPtr boxColliderPtr = stencilObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(
		glm::vec3(
			boxConfig->getBoxXSize() * 0.5f,
			boxConfig->getBoxYSize() * 0.5f,
			boxConfig->getBoxZSize() * 0.5f));
	boxColliderPtr->attachToComponent(stencilComponentPtr);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Init the object once all components are added
	stencilObject->init();

	// Keep track of all the box stencils in the stencil system
	m_boxStencilComponents.insert({boxConfig->getStencilId(), stencilComponentPtr});

	return stencilComponentPtr;
}

bool StencilObjectSystem::disposeBoxStencilObject(MikanStencilID stencilId)
{
	auto it = m_boxStencilComponents.find(stencilId);
	if (it != m_boxStencilComponents.end())
	{
		BoxStencilComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_boxStencilComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}

ModelStencilComponentPtr StencilObjectSystem::getModelStencilById(MikanStencilID stencilId) const
{
	auto iter = m_modelStencilComponents.find(stencilId);
	if (iter != m_modelStencilComponents.end())
	{
		return iter->second.lock();
	}

	return ModelStencilComponentPtr();
}

ModelStencilComponentPtr StencilObjectSystem::getModelStencilByName(const std::string& stencilName) const
{
	for (auto it = m_modelStencilComponents.begin(); it != m_modelStencilComponents.end(); it++)
	{
		ModelStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == stencilName)
		{
			return componentPtr;
		}
	}

	return ModelStencilComponentPtr();
}

ModelStencilComponentPtr StencilObjectSystem::addNewModelStencil(const MikanStencilModel& stencilInfo)
{
	StencilObjectSystemConfigPtr stencilSystemConfig = getStencilSystemConfig();

	MikanStencilID stencilId = stencilSystemConfig->addNewModelStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		ModelStencilDefinitionPtr configPtr = stencilSystemConfig->getModelStencilConfig(stencilId);
		assert(configPtr != nullptr);

		return createModelStencilObject(configPtr);
	}

	return ModelStencilComponentPtr();
}

bool StencilObjectSystem::removeModelStencil(MikanStencilID stencilId)
{
	return 
		disposeModelStencilObject(stencilId) &&
		getStencilSystemConfig()->removeStencil(stencilId);
}

void StencilObjectSystem::getRelevantModelStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	std::vector<ModelStencilComponentPtr>& outStencilList) const
{
	outStencilList.clear();
	for (auto it = m_modelStencilComponents.begin(); it != m_modelStencilComponents.end(); it++)
	{
		MikanStencilID stencilId = it->first;
		ModelStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr->getModelStencilDefinition()->getIsDisabled())
			continue;

		if (componentPtr->getModelStencilDefinition()->getModelPath().empty())
			continue;

		// If there is an active allow list, make sure stencil is on it
		if (allowedStencilIds != nullptr)
		{
			if (std::find(allowedStencilIds->begin(), allowedStencilIds->end(), stencilId) == allowedStencilIds->end())
			{
				continue;
			}
		}

		outStencilList.push_back(componentPtr);
	}
}

ModelStencilComponentPtr StencilObjectSystem::createModelStencilObject(ModelStencilDefinitionPtr modelConfig)
{
	MikanObjectPtr stencilObject = newObject();
	stencilObject->setName(modelConfig->getComponentName());

	// Make the model stencil component the root object
	ModelStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<ModelStencilComponent>();
	stencilObject->setRootComponent(stencilComponentPtr);
	// Setting the config will spawn child mesh componets, if any
	stencilComponentPtr->setDefinition(modelConfig);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Init the object once all components are added
	stencilObject->init();

	// Create mesh mesh components for the assigned mesh target
	stencilComponentPtr->rebuildMeshComponents();

	// Add the model stencil to the list of stencils
	m_modelStencilComponents.insert({modelConfig->getStencilId(), stencilComponentPtr});

	return stencilComponentPtr;
}

bool StencilObjectSystem::disposeModelStencilObject(MikanStencilID stencilId)
{
	auto it = m_modelStencilComponents.find(stencilId);
	if (it != m_modelStencilComponents.end())
	{
		ModelStencilComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_modelStencilComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}

StencilObjectSystemConfigConstPtr StencilObjectSystem::getStencilSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->stencilConfig;
}

StencilObjectSystemConfigPtr StencilObjectSystem::getStencilSystemConfig()
{
	return std::const_pointer_cast<StencilObjectSystemConfig>(getStencilSystemConfigConst());
}