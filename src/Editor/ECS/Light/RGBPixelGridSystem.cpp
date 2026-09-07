#include "RGBPixelGridSystem.h"
#include "BoxColliderComponent.h"
#include "MikanObject.h"
#include "SelectionComponent.h"

// -- RGBPixelGridSystemDefinition -----
RGBPixelGridSystemDefinition::RGBPixelGridSystemDefinition(const std::string& configName,
														   IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- RGBPixelGridSystem -----
RGBPixelGridSystem::RGBPixelGridSystem(ProjectManagerPtr ownerProjectManager)
	: Super::MikanTypedObjectSystem(ownerProjectManager)
{
}

void RGBPixelGridSystem::additionalComponentFactory(MikanObjectPtr ownerComponentObject,
													ComponentDefinitionPtr componentDefinition)
{
	// Add a selection component so the grid can be clicked in the viewport
	ownerComponentObject->addComponent<SelectionComponent>();

	RGBPixelGridComponentPtr gridComponentPtr= ownerComponentObject->getComponentOfType<RGBPixelGridComponent>();
	if (gridComponentPtr)
	{
		// One box around the whole grid rather than one per pixel
		BoxColliderComponentPtr boxColliderPtr= ownerComponentObject->addComponent<BoxColliderComponent>();
		boxColliderPtr->attachToComponent(gridComponentPtr);
	}
}