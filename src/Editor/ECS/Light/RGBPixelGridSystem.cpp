#include "RGBPixelGridSystem.h"
#
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