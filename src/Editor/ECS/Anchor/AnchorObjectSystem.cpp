#include "App.h"
#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "BoxColliderComponent.h"
#include "TransformComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- AnchorObjectSystemDefinition -----
const std::string AnchorObjectSystemDefinition::k_renderAnchorsPropertyId = "render_anchors";

AnchorObjectSystemDefinition::AnchorObjectSystemDefinition(
	const std::string& configName,
	IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

configuru::Config AnchorObjectSystemDefinition::writeToJSON()
{
	configuru::Config pt = Super::writeToJSON();

	pt["debugRenderAnchors"] = m_bDebugRenderAnchors;

	return pt;
}

void AnchorObjectSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	Super::readFromJSON(pt);

	m_bDebugRenderAnchors = pt.get_or<bool>("debugRenderAnchors", m_bDebugRenderAnchors);
}

void AnchorObjectSystemDefinition::setRenderAnchorsFlag(bool flag)
{
	if (m_bDebugRenderAnchors != flag)
	{
		m_bDebugRenderAnchors = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderAnchorsPropertyId));
	}
}

// -- AnchorObjectSystem -----
AnchorObjectSystem::AnchorObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

bool AnchorObjectSystem::getSpatialAnchorWorldTransform(MikanSpatialAnchorID anchorId, glm::mat4& outXform) const
{
	AnchorComponentPtr anchorPtr = getSpatialAnchorById(anchorId);
	if (anchorPtr)
	{
		outXform = anchorPtr->getWorldTransform();
		return true;
	}

	return false;
}

void AnchorObjectSystem::additionalComponentFactory(
	MikanObjectPtr ownerComponentObject,
	ComponentDefinitionPtr componentDefinition)
{
	TransformComponentPtr rootComponent = ownerComponentObject->getRootComponent();
	assert(rootComponent);

	// Add a selection component
	ownerComponentObject->addComponent<SelectionComponent>();

	// Attach a box collider to anchor component
	const float size = 0.1f;
	BoxColliderComponentPtr boxColliderPtr = ownerComponentObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(glm::vec3(size * 0.5f, size * 0.5f, size * 0.5f));
	boxColliderPtr->setRelativeTransform(GlmTransform(glm::vec3(size * 0.5f, size * 0.5f, size * 0.5f)));
	boxColliderPtr->attachToComponent(rootComponent);
}

// -- IPropertyInterface ----
void AnchorObjectSystem::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanObjectSystem::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			AnchorObjectSystemDefinition::k_renderAnchorsPropertyId, MikanVariantType::BOOL));
}

bool AnchorObjectSystem::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	if (propertyName == AnchorObjectSystemDefinition::k_renderAnchorsPropertyId)
	{
		AnchorObjectSystemDefinitionConstPtr definition = getTypedDefinitionConst();
		outValue = definition->getRenderAnchorsFlag();
		return true;
	}

	return MikanObjectSystem::getPropertyValue(propertyName, outValue);
}

bool AnchorObjectSystem::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	if (propertyName == AnchorObjectSystemDefinition::k_renderAnchorsPropertyId)
	{
		AnchorObjectSystemDefinitionPtr definition = getTypedDefinition();
		definition->setRenderAnchorsFlag(inValue.getBoolValue());
		return true;
	}

	return MikanObjectSystem::setPropertyValue(propertyName, inValue);
}