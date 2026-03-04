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
const std::string ModelStencilSystemDefinition::k_renderStencilsPropertyId = "render_stencils";

ModelStencilSystemDefinition::ModelStencilSystemDefinition(const std::string& configName)
	: Super::MikanTypedObjectSystemDefinition(configName)
{
}

configuru::Config ModelStencilSystemDefinition::writeToJSON()
{
	configuru::Config pt = Super::writeToJSON();

	pt["debug_render_stencils"] = m_bDebugRenderStencils;

	return pt;
}

void ModelStencilSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	Super::readFromJSON(pt);

	m_bDebugRenderStencils = pt.get_or<bool>("debug_render_stencils", m_bDebugRenderStencils);
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
ModelStencilSystem::ModelStencilSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

void ModelStencilSystem::getRelevantModelStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<ModelStencilComponentPtr>& outStencilList) const
{
	outStencilList.clear();
	for (const auto& stencilPair : Super::getComponentMap())
	{
		MikanStencilID stencilId = stencilPair.first;
		ModelStencilComponentPtr componentPtr = stencilPair.second.lock();

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

void ModelStencilSystem::additionalComponentFactory(
	MikanObjectPtr ownerComponentObject,
	ComponentDefinitionPtr componentDefinition)
{
	// Add a selection component
	ownerComponentObject->addComponent<SelectionComponent>();

	// Get the model stencil component that was just created
	ModelStencilComponentPtr stencilComponentPtr = ownerComponentObject->getComponentOfType<ModelStencilComponent>();

	// Create mesh components for the assigned mesh target
	// This needs to happen after init, so we do it in the init callback
	if (stencilComponentPtr)
	{
		stencilComponentPtr->rebuildMeshComponents();
	}
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

// -- IPropertyInterface ----
void ModelStencilSystem::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanObjectSystem::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			ModelStencilSystemDefinition::k_renderStencilsPropertyId, MikanVariantType::BOOL));
}

bool ModelStencilSystem::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	if (propertyName == ModelStencilSystemDefinition::k_renderStencilsPropertyId)
	{
		ModelStencilSystemDefinitionConstPtr definition = getTypedDefinitionConst();
		outValue = definition->getRenderStencilsFlag();
		return true;
	}

	return MikanObjectSystem::getPropertyValue(propertyName, outValue);
}

bool ModelStencilSystem::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	if (propertyName == ModelStencilSystemDefinition::k_renderStencilsPropertyId)
	{
		ModelStencilSystemDefinitionPtr definition = getTypedDefinition();
		definition->setRenderStencilsFlag(inValue.getBoolValue());
		return true;
	}

	return MikanObjectSystem::setPropertyValue(propertyName, inValue);
}
