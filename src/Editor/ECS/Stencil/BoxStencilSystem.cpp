#include "BoxStencilSystem.h"
#include "BoxStencilComponent.h"
#include "BoxColliderComponent.h"
#include "MathTypeConversion.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"

#include <assert.h>

// -- BoxStencilSystemDefinition -----
const std::string BoxStencilSystemDefinition::k_boxStencilListPropertyId = "box_stencils";
const std::string BoxStencilSystemDefinition::k_renderStencilsPropertyId = "render_stencils";

configuru::Config BoxStencilSystemDefinition::writeToJSON()
{
	configuru::Config pt = MikanObjectSystemDefinition::writeToJSON();

	pt["next_stencil_id"] = m_nextStencilId;
	pt["debug_render_stencils"] = m_bDebugRenderStencils;

	std::vector<configuru::Config> boxStencilConfigs;
	for (auto boxStencil : m_boxStencilList)
	{
		boxStencilConfigs.push_back(boxStencil->writeToJSON());
	}
	pt.insert_or_assign(std::string("box_stencils"), boxStencilConfigs);

	return pt;
}

void BoxStencilSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanObjectSystemDefinition::readFromJSON(pt);

	m_nextStencilId = pt.get_or<int>("next_stencil_id", m_nextStencilId);
	m_bDebugRenderStencils = pt.get_or<bool>("debug_render_stencils", m_bDebugRenderStencils);

	// Read in the box stencils
	m_boxStencilList.clear();
	if (pt.has_key("box_stencils"))
	{
		for (const configuru::Config& stencilConfig : pt["box_stencils"].as_array())
		{
			auto definitionPtr = std::make_shared<BoxStencilDefinition>();

			definitionPtr->readFromJSON(stencilConfig);
			m_boxStencilList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

BoxStencilDefinitionConstPtr BoxStencilSystemDefinition::getBoxStencilConfigConst(
	MikanStencilID stencilId) const
{
	auto it = std::find_if(
		m_boxStencilList.begin(), m_boxStencilList.end(),
		[stencilId](BoxStencilDefinitionPtr configPtr) {
			return configPtr->getStencilId() == stencilId;
		});

	if (it != m_boxStencilList.end())
	{
		return BoxStencilDefinitionConstPtr(*it);
	}

	return BoxStencilDefinitionConstPtr();
}

BoxStencilDefinitionPtr BoxStencilSystemDefinition::getBoxStencilConfig(
	MikanStencilID stencilId)
{
	return std::const_pointer_cast<BoxStencilDefinition>(
		getBoxStencilConfigConst(stencilId));
}

BoxStencilDefinitionPtr BoxStencilSystemDefinition::allocateBoxStencilDefinition(
	const MikanStencilBoxInfo& boxInfo)
{
	MikanStencilBoxInfo localBoxInfo = boxInfo;
	localBoxInfo.stencil_id = m_nextStencilId;

	BoxStencilDefinitionPtr boxDefinitionPtr =
		std::make_shared<BoxStencilDefinition>(localBoxInfo);
	m_nextStencilId++;

	return boxDefinitionPtr;
}

bool BoxStencilSystemDefinition::addBoxStencilDefinition(
	BoxStencilDefinitionPtr boxDefinitionPtr)
{
	if (!getBoxStencilConfig(boxDefinitionPtr->getStencilId()))
	{
		m_boxStencilList.push_back(boxDefinitionPtr);
		addChildConfig(boxDefinitionPtr);

		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_boxStencilListPropertyId));
		return true;
	}

	return false;
}

bool BoxStencilSystemDefinition::removeBoxStencilDefinition(MikanStencilID stencilId)
{
	auto it = std::find_if(
		m_boxStencilList.begin(), m_boxStencilList.end(),
		[stencilId](BoxStencilDefinitionPtr configPtr) {
			return configPtr->getStencilId() == stencilId;
		});

	if (it != m_boxStencilList.end())
	{
		removeChildConfig(*it);

		m_boxStencilList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_boxStencilListPropertyId));

		return true;
	}

	return false;
}

void BoxStencilSystemDefinition::setRenderStencilsFlag(bool flag)
{
	if (m_bDebugRenderStencils != flag)
	{
		m_bDebugRenderStencils = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderStencilsPropertyId));
	}
}

// -- BoxStencilSystem ----
BoxStencilSystemDefinitionConstPtr BoxStencilSystem::getBoxStencilSystemDefinitionConst() const
{
	return std::static_pointer_cast<const BoxStencilSystemDefinition>(getDefinitionConst());
}

BoxStencilSystemDefinitionPtr BoxStencilSystem::getBoxStencilSystemDefinition()
{
	return std::static_pointer_cast<BoxStencilSystemDefinition>(getDefinition());
}

bool BoxStencilSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

	BoxStencilSystemDefinitionConstPtr boxStencilSystemDefinition =
		getProjectConfig()->boxStencilSystemDefinition;

	for (const auto& boxConfig : boxStencilSystemDefinition->getBoxStencilList())
	{
		createBoxStencilObject(boxConfig);
	}

	return true;
}

void BoxStencilSystem::dispose()
{
	m_boxStencilComponents.clear();
	MikanObjectSystem::dispose();
}

void BoxStencilSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	BoxStencilComponentPtr boxStencil = objectPtr->getComponentOfType<BoxStencilComponent>();
	if (boxStencil != nullptr)
	{
		removeBoxStencil(boxStencil->getStencilComponentDefinition()->getStencilId());
	}
}

MikanComponentPtr BoxStencilSystem::getComponentById(int componentId) const
{
	return getBoxStencilById(componentId);
}

BoxStencilComponentPtr BoxStencilSystem::getBoxStencilById(MikanStencilID stencilId) const
{
	auto iter = m_boxStencilComponents.find(stencilId);
	if (iter != m_boxStencilComponents.end())
	{
		return iter->second.lock();
	}

	return BoxStencilComponentPtr();
}

BoxStencilComponentPtr BoxStencilSystem::getBoxStencilByName(const std::string& stencilName) const
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

BoxStencilComponentPtr BoxStencilSystem::addNewBoxStencil(const MikanStencilBoxInfo& stencilInfo)
{
	BoxStencilSystemDefinitionPtr boxStencilSystemDefinition = getProjectConfig()->boxStencilSystemDefinition;

	return
		createBoxStencilObject(
			boxStencilSystemDefinition->allocateBoxStencilDefinition(stencilInfo));
}

bool BoxStencilSystem::removeBoxStencil(MikanStencilID stencilId)
{
	BoxStencilSystemDefinitionPtr boxStencilSystemDefinition =
		getProjectConfig()->boxStencilSystemDefinition;

	return
		disposeBoxStencilObject(stencilId) &&
		boxStencilSystemDefinition->removeBoxStencilDefinition(stencilId);
}

void BoxStencilSystem::getRelevantBoxStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<BoxStencilComponentPtr>& outStencilList) const
{
	outStencilList.clear();
	for (auto it = m_boxStencilComponents.begin(); it != m_boxStencilComponents.end(); it++)
	{
		MikanSpatialAnchorID stencilId = it->first;
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

		if (!isStencilFacingCamera(componentPtr, cameraPosition, cameraForward))
			continue;

		{
			const glm::mat4 worldXform = componentPtr->getWorldTransform();
			const glm::vec3 stencilCenter = glm::vec3(worldXform[3]); // position is 3rd column
			const glm::vec3 stencilZAxis = glm::vec3(worldXform[2]); // Z is 2nd column
			const glm::vec3 stencilYAxis = glm::vec3(worldXform[1]); // Y is 1st column
			const glm::vec3 stencilXAxis = glm::vec3(worldXform[0]); // X is 0th column
			BoxStencilDefinitionConstPtr configPtr = componentPtr->getBoxStencilDefinition();
			const float boxXSize = configPtr->getBoxXSize();
			const float boxYSize = configPtr->getBoxYSize();
			const float boxZSize = configPtr->getBoxZSize();
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

BoxStencilComponentPtr BoxStencilSystem::createBoxStencilObject(BoxStencilDefinitionPtr boxConfig)
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
	m_boxStencilComponents.insert({ boxConfig->getStencilId(), stencilComponentPtr });

	// Register the definition with the box stencil system
	getProjectConfig()->boxStencilSystemDefinition->addBoxStencilDefinition(boxConfig);

	return stencilComponentPtr;
}

bool BoxStencilSystem::disposeBoxStencilObject(MikanStencilID stencilId)
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

bool BoxStencilSystem::isStencilFacingCamera(
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

void BoxStencilSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<BoxStencilSystem>();
	propertyDatabase->registerPropertiesForComponent<BoxStencilSystem, BoxStencilComponent>();
}
