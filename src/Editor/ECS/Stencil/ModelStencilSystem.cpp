#include "ModelStencilSystem.h"
#include "ModelStencilComponent.h"
#include "MathTypeConversion.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"

#include <assert.h>

// -- ModelStencilSystemDefinition -----
const std::string ModelStencilSystemDefinition::k_modelStencilListPropertyId = "model_stencils";
const std::string ModelStencilSystemDefinition::k_renderStencilsPropertyId = "render_stencils";

configuru::Config ModelStencilSystemDefinition::writeToJSON()
{
	configuru::Config pt = MikanObjectSystemDefinition::writeToJSON();

	pt["next_stencil_id"] = m_nextStencilId;
	pt["debug_render_stencils"] = m_bDebugRenderStencils;

	std::vector<configuru::Config> modelStencilConfigs;
	for (auto modelStencil : m_modelStencilList)
	{
		modelStencilConfigs.push_back(modelStencil->writeToJSON());
	}
	pt.insert_or_assign(std::string("model_stencils"), modelStencilConfigs);

	return pt;
}

void ModelStencilSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanObjectSystemDefinition::readFromJSON(pt);

	m_nextStencilId = pt.get_or<int>("next_stencil_id", m_nextStencilId);
	m_bDebugRenderStencils = pt.get_or<bool>("debug_render_stencils", m_bDebugRenderStencils);

	// Read in the model stencils
	m_modelStencilList.clear();
	if (pt.has_key("model_stencils"))
	{
		for (const configuru::Config& stencilConfig : pt["model_stencils"].as_array())
		{
			auto definitionPtr = std::make_shared<ModelStencilDefinition>();

			definitionPtr->readFromJSON(stencilConfig);
			m_modelStencilList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

ModelStencilDefinitionConstPtr ModelStencilSystemDefinition::getModelStencilConfigConst(
	MikanStencilID stencilId) const
{
	auto it = std::find_if(
		m_modelStencilList.begin(), m_modelStencilList.end(),
		[stencilId](ModelStencilDefinitionPtr configPtr) {
			return configPtr->getStencilId() == stencilId;
		});

	if (it != m_modelStencilList.end())
	{
		return ModelStencilDefinitionConstPtr(*it);
	}

	return ModelStencilDefinitionConstPtr();
}

ModelStencilDefinitionPtr ModelStencilSystemDefinition::getModelStencilConfig(
	MikanStencilID stencilId)
{
	return std::const_pointer_cast<ModelStencilDefinition>(
		getModelStencilConfigConst(stencilId));
}

ModelStencilDefinitionPtr ModelStencilSystemDefinition::allocateModelStencilDefinition(
	const MikanStencilModelInfo& modelInfo)
{
	MikanStencilModelInfo localModelInfo = modelInfo;
	localModelInfo.stencil_id = m_nextStencilId;

	ModelStencilDefinitionPtr modelDefinitionPtr =
		std::make_shared<ModelStencilDefinition>(localModelInfo);
	m_nextStencilId++;

	return modelDefinitionPtr;
}

bool ModelStencilSystemDefinition::addModelStencilDefinition(
	ModelStencilDefinitionPtr modelDefinitionPtr)
{
	if (!getModelStencilConfig(modelDefinitionPtr->getStencilId()))
	{
		m_modelStencilList.push_back(modelDefinitionPtr);
		addChildConfig(modelDefinitionPtr);

		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_modelStencilListPropertyId));
		return true;
	}

	return false;
}

bool ModelStencilSystemDefinition::removeModelStencilDefinition(MikanStencilID stencilId)
{
	auto it = std::find_if(
		m_modelStencilList.begin(), m_modelStencilList.end(),
		[stencilId](ModelStencilDefinitionPtr configPtr) {
			return configPtr->getStencilId() == stencilId;
		});

	if (it != m_modelStencilList.end())
	{
		removeChildConfig(*it);

		m_modelStencilList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_modelStencilListPropertyId));

		return true;
	}

	return false;
}

void ModelStencilSystemDefinition::setRenderStencilsFlag(bool flag)
{
	if (m_bDebugRenderStencils != flag)
	{
		m_bDebugRenderStencils = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderStencilsPropertyId));
	}
}

// -- ModelStencilSystem ----
ModelStencilSystemDefinitionConstPtr ModelStencilSystem::getModelStencilSystemDefinitionConst() const
{
	return std::static_pointer_cast<const ModelStencilSystemDefinition>(getDefinitionConst());
}

ModelStencilSystemDefinitionPtr ModelStencilSystem::getModelStencilSystemDefinition()
{
	return std::static_pointer_cast<ModelStencilSystemDefinition>(getDefinition());
}

bool ModelStencilSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

	ModelStencilSystemDefinitionConstPtr modelStencilSystemDefinition =
		getProjectConfig()->modelStencilSystemDefinition;

	for (const auto& modelConfig : modelStencilSystemDefinition->getModelStencilList())
	{
		createModelStencilObject(modelConfig);
	}

	return true;
}

void ModelStencilSystem::dispose()
{
	m_modelStencilComponents.clear();
	MikanObjectSystem::dispose();
}

void ModelStencilSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	ModelStencilComponentPtr modelStencil = objectPtr->getComponentOfType<ModelStencilComponent>();
	if (modelStencil != nullptr)
	{
		removeModelStencil(modelStencil->getStencilComponentDefinition()->getStencilId());
	}
}

MikanComponentPtr ModelStencilSystem::getComponentById(int componentId) const
{
	return getModelStencilById(componentId);
}

ModelStencilComponentPtr ModelStencilSystem::getModelStencilById(MikanStencilID stencilId) const
{
	auto iter = m_modelStencilComponents.find(stencilId);
	if (iter != m_modelStencilComponents.end())
	{
		return iter->second.lock();
	}

	return ModelStencilComponentPtr();
}

ModelStencilComponentPtr ModelStencilSystem::getModelStencilByName(const std::string& stencilName) const
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

ModelStencilComponentPtr ModelStencilSystem::addNewModelStencil(const MikanStencilModelInfo& stencilInfo)
{
	ModelStencilSystemDefinitionPtr modelStencilSystemDefinition = getProjectConfig()->modelStencilSystemDefinition;

	return
		createModelStencilObject(
			modelStencilSystemDefinition->allocateModelStencilDefinition(stencilInfo));
}

bool ModelStencilSystem::removeModelStencil(MikanStencilID stencilId)
{
	ModelStencilSystemDefinitionPtr modelStencilSystemDefinition =
		getProjectConfig()->modelStencilSystemDefinition;

	return
		disposeModelStencilObject(stencilId) &&
		modelStencilSystemDefinition->removeModelStencilDefinition(stencilId);
}

void ModelStencilSystem::getRelevantModelStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
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

		if (!isStencilFacingCamera(componentPtr, cameraPosition, cameraForward))
			continue;

		outStencilList.push_back(componentPtr);
	}
}

void ModelStencilSystem::setRenderStencilsFlag(bool flag)
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
	getModelStencilSystemDefinition()->setRenderStencilsFlag(flag);
}

ModelStencilComponentPtr ModelStencilSystem::createModelStencilObject(ModelStencilDefinitionPtr modelConfig)
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
	m_modelStencilComponents.insert({ modelConfig->getStencilId(), stencilComponentPtr });

	// Register the definition with the model stencil system
	getProjectConfig()->modelStencilSystemDefinition->addModelStencilDefinition(modelConfig);

	return stencilComponentPtr;
}

bool ModelStencilSystem::disposeModelStencilObject(MikanStencilID stencilId)
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

bool ModelStencilSystem::isStencilFacingCamera(
	StencilComponentConstPtr stencil,
	const glm::vec3& cameraPosition, const glm::vec3& cameraForward)
{
	StencilComponentConfigConstPtr configPtr = stencil->getStencilComponentDefinition();
	eStencilCullMode cullMode = configPtr->getCullMode();

	if (cullMode == eStencilCullMode::none)
		return true;

	glm::mat4 stencilXform = stencil->getWorldTransform();
	glm::vec3 stencilCenter = glm_mat4_get_position(stencilXform);
	glm::vec3 stencilForward;
	switch (cullMode)
	{
	case eStencilCullMode::zAxis:
		stencilForward = glm_mat4_get_z_axis(stencilXform);
		break;
	case eStencilCullMode::yAxis:
		stencilForward = glm_mat4_get_y_axis(stencilXform);
		break;
	case eStencilCullMode::xAxis:
		stencilForward = glm_mat4_get_x_axis(stencilXform);
		break;
	}

	const glm::vec3 cameraToStencil = stencilCenter - cameraPosition;
	const glm::vec3 stencilToCamera = -cameraToStencil;

	return
		glm::dot(cameraToStencil, cameraForward) > 0.f &&
		glm::dot(stencilToCamera, stencilForward) > 0.f;
}

void ModelStencilSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<ModelStencilSystem>();
	propertyDatabase->registerPropertiesForComponent<ModelStencilSystem, ModelStencilComponent>();
}
