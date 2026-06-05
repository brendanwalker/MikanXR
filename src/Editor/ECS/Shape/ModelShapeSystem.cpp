#include "ModelShapeSystem.h"
#include "MikanObject.h"
#include "MikanShapeTypes.h"
#include "SelectionComponent.h"

#include <assert.h>

// -- ModelShapeSystemDefinition -----
ModelShapeSystemDefinition::ModelShapeSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

configuru::Config ModelShapeSystemDefinition::writeToJSON()
{
	return Super::writeToJSON();
}

void ModelShapeSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	Super::readFromJSON(pt);
}

// -- ModelShapeSystem ----
ModelShapeSystem::ModelShapeSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

// -- IEntityAccessor ----
rfk::Struct const* ModelShapeSystem::getClientAPIValuesStructType() const
{
	return &MikanModelShapeSystemValues::staticGetArchetype();
}

void ModelShapeSystem::getModelShapeComponentList(std::vector<ModelShapeComponentPtr>& outList) const
{
	outList.clear();
	for (const auto& pair : Super::getComponentMap())
	{
		ModelShapeComponentPtr comp = pair.second.lock();
		if (comp)
			outList.push_back(comp);
	}
}

void ModelShapeSystem::additionalComponentFactory(
	MikanObjectPtr ownerComponentObject,
	ComponentDefinitionPtr componentDefinition)
{
	// SelectionComponent for picking in the editor
	ownerComponentObject->addComponent<SelectionComponent>();
	// Note: mesh colliders are created by ModelShapeComponent::rebuildMeshComponents()
}
