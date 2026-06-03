#include "QuadShapeSystem.h"
#include "BoxColliderComponent.h"
#include "MikanObject.h"
#include "SelectionComponent.h"

#include <assert.h>

// -- QuadShapeSystemDefinition -----
QuadShapeSystemDefinition::QuadShapeSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

configuru::Config QuadShapeSystemDefinition::writeToJSON()
{
	return Super::writeToJSON();
}

void QuadShapeSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	Super::readFromJSON(pt);
}

// -- QuadShapeSystem ----
QuadShapeSystem::QuadShapeSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

void QuadShapeSystem::getQuadShapeComponentList(std::vector<QuadShapeComponentPtr>& outList) const
{
	outList.clear();
	for (const auto& pair : Super::getComponentMap())
	{
		QuadShapeComponentPtr comp = pair.second.lock();
		if (comp)
			outList.push_back(comp);
	}
}

void QuadShapeSystem::additionalComponentFactory(
	MikanObjectPtr ownerComponentObject,
	ComponentDefinitionPtr componentDefinition)
{
	TransformComponentPtr rootComponent = ownerComponentObject->getRootComponent();
	assert(rootComponent);

	QuadShapeDefinitionPtr quadDef =
		std::static_pointer_cast<QuadShapeDefinition>(componentDefinition);

	// Attach box collider sized to the quad
	BoxColliderComponentPtr boxCollider = ownerComponentObject->addComponent<BoxColliderComponent>();
	boxCollider->setHalfExtents(
		glm::vec3(quadDef->getQuadWidth() * 0.5f, quadDef->getQuadHeight() * 0.5f, 0.01f));
	boxCollider->attachToComponent(rootComponent);

	// Attach selection component
	ownerComponentObject->addComponent<SelectionComponent>();
}
