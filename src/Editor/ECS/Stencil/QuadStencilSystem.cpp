#include "QuadStencilSystem.h"
#include "QuadStencilComponent.h"
#include "BoxColliderComponent.h"
#include "MathTypeConversion.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"

#include <assert.h>

// -- QuadStencilSystemDefinition -----
const std::string QuadStencilSystemDefinition::k_quadStencilListPropertyId = "quad_stencils";
const std::string QuadStencilSystemDefinition::k_renderStencilsPropertyId = "render_stencils";

configuru::Config QuadStencilSystemDefinition::writeToJSON()
{
	configuru::Config pt = MikanObjectSystemDefinition::writeToJSON();

	pt["next_stencil_id"] = m_nextStencilId;
	pt["debug_render_stencils"] = m_bDebugRenderStencils;

	std::vector<configuru::Config> quadStencilConfigs;
	for (auto quadStencil : m_quadStencilList)
	{
		quadStencilConfigs.push_back(quadStencil->writeToJSON());
	}
	pt.insert_or_assign(std::string("quad_stencils"), quadStencilConfigs);

	return pt;
}

void QuadStencilSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanObjectSystemDefinition::readFromJSON(pt);

	m_nextStencilId = pt.get_or<int>("next_stencil_id", m_nextStencilId);
	m_bDebugRenderStencils = pt.get_or<bool>("debug_render_stencils", m_bDebugRenderStencils);

	// Read in the quad stencils
	m_quadStencilList.clear();
	if (pt.has_key("quad_stencils"))
	{
		for (const configuru::Config& stencilConfig : pt["quad_stencils"].as_array())
		{
			auto definitionPtr = std::make_shared<QuadStencilDefinition>();

			definitionPtr->readFromJSON(stencilConfig);
			m_quadStencilList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

QuadStencilDefinitionConstPtr QuadStencilSystemDefinition::getQuadStencilConfigConst(
	MikanStencilID stencilId) const
{
	auto it = std::find_if(
		m_quadStencilList.begin(), m_quadStencilList.end(),
		[stencilId](QuadStencilDefinitionPtr configPtr) {
			return configPtr->getStencilId() == stencilId;
		});

	if (it != m_quadStencilList.end())
	{
		return QuadStencilDefinitionConstPtr(*it);
	}

	return QuadStencilDefinitionConstPtr();
}

QuadStencilDefinitionPtr QuadStencilSystemDefinition::getQuadStencilConfig(
	MikanStencilID stencilId)
{
	return std::const_pointer_cast<QuadStencilDefinition>(
		getQuadStencilConfigConst(stencilId));
}

QuadStencilDefinitionPtr QuadStencilSystemDefinition::allocateQuadStencilDefinition(
	const MikanStencilQuadInfo& quadInfo)
{
	MikanStencilQuadInfo localQuadInfo = quadInfo;
	localQuadInfo.stencil_id = m_nextStencilId;

	QuadStencilDefinitionPtr quadDefinitionPtr =
		std::make_shared<QuadStencilDefinition>(localQuadInfo);
	m_nextStencilId++;

	return quadDefinitionPtr;
}

bool QuadStencilSystemDefinition::addQuadStencilDefinition(
	QuadStencilDefinitionPtr quadDefinitionPtr)
{
	if (!getQuadStencilConfig(quadDefinitionPtr->getStencilId()))
	{
		m_quadStencilList.push_back(quadDefinitionPtr);
		addChildConfig(quadDefinitionPtr);

		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_quadStencilListPropertyId));
		return true;
	}

	return false;
}

bool QuadStencilSystemDefinition::removeQuadStencilDefinition(MikanStencilID stencilId)
{
	auto it = std::find_if(
		m_quadStencilList.begin(), m_quadStencilList.end(),
		[stencilId](QuadStencilDefinitionPtr configPtr) {
			return configPtr->getStencilId() == stencilId;
		});

	if (it != m_quadStencilList.end())
	{
		removeChildConfig(*it);

		m_quadStencilList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_quadStencilListPropertyId));

		return true;
	}

	return false;
}

void QuadStencilSystemDefinition::setRenderStencilsFlag(bool flag)
{
	if (m_bDebugRenderStencils != flag)
	{
		m_bDebugRenderStencils = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderStencilsPropertyId));
	}
}

// -- QuadStencilSystem ----
QuadStencilSystemDefinitionConstPtr QuadStencilSystem::getQuadStencilSystemDefinitionConst() const
{
	return std::static_pointer_cast<const QuadStencilSystemDefinition>(getDefinitionConst());
}

QuadStencilSystemDefinitionPtr QuadStencilSystem::getQuadStencilSystemDefinition()
{
	return std::static_pointer_cast<QuadStencilSystemDefinition>(getDefinition());
}

bool QuadStencilSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

	QuadStencilSystemDefinitionConstPtr quadStencilSystemDefinition =
		getProjectConfig()->quadStencilSystemDefinition;

	for (const auto& quadConfig : quadStencilSystemDefinition->getQuadStencilList())
	{
		createQuadStencilObject(quadConfig);
	}

	return true;
}

void QuadStencilSystem::dispose()
{
	m_quadStencilComponents.clear();
	MikanObjectSystem::dispose();
}

void QuadStencilSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	QuadStencilComponentPtr quadStencil = objectPtr->getComponentOfType<QuadStencilComponent>();
	if (quadStencil != nullptr)
	{
		removeQuadStencil(quadStencil->getStencilComponentDefinition()->getStencilId());
	}
}

MikanComponentPtr QuadStencilSystem::getComponentById(int componentId) const
{
	return getQuadStencilById(componentId);
}

QuadStencilComponentPtr QuadStencilSystem::getQuadStencilById(MikanStencilID stencilId) const
{
	auto iter = m_quadStencilComponents.find(stencilId);
	if (iter != m_quadStencilComponents.end())
	{
		return iter->second.lock();
	}

	return QuadStencilComponentPtr();
}

QuadStencilComponentPtr QuadStencilSystem::getQuadStencilByName(const std::string& stencilName) const
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

QuadStencilComponentPtr QuadStencilSystem::addNewQuadStencil(const MikanStencilQuadInfo& stencilInfo)
{
	QuadStencilSystemDefinitionPtr quadStencilSystemDefinition = getProjectConfig()->quadStencilSystemDefinition;

	return
		createQuadStencilObject(
			quadStencilSystemDefinition->allocateQuadStencilDefinition(stencilInfo));
}

bool QuadStencilSystem::removeQuadStencil(MikanStencilID stencilId)
{
	QuadStencilSystemDefinitionPtr quadStencilSystemDefinition =
		getProjectConfig()->quadStencilSystemDefinition;

	return
		disposeQuadStencilObject(stencilId) &&
		quadStencilSystemDefinition->removeQuadStencilDefinition(stencilId);
}

void QuadStencilSystem::getRelevantQuadStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<QuadStencilComponentPtr>& outStencilList) const
{
	outStencilList.clear();
	for (auto it = m_quadStencilComponents.begin(); it != m_quadStencilComponents.end(); it++)
	{
		MikanStencilID stencilId = it->first;
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

		if (!isStencilFacingCamera(componentPtr, cameraPosition, cameraForward))
			continue;

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

QuadStencilComponentPtr QuadStencilSystem::createQuadStencilObject(QuadStencilDefinitionPtr quadConfig)
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
	m_quadStencilComponents.insert({ quadConfig->getStencilId(), stencilComponentPtr });

	// Register the definition with the quad stencil system
	getProjectConfig()->quadStencilSystemDefinition->addQuadStencilDefinition(quadConfig);

	return stencilComponentPtr;
}

bool QuadStencilSystem::disposeQuadStencilObject(MikanStencilID stencilId)
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

bool QuadStencilSystem::isStencilFacingCamera(
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

void QuadStencilSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<QuadStencilSystem>();
	propertyDatabase->registerPropertiesForComponent<QuadStencilSystem, QuadStencilComponent>();
}
