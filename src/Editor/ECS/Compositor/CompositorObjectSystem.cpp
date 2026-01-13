#include "App.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "ProjectConfig.h"

// -- CompositorObjectSystemDefinition -----
CompositorObjectSystemDefinition::CompositorObjectSystemDefinition(const std::string& configName)
	: Super::MikanTypedObjectSystemDefinition(configName)
{
}

// -- CompositorObjectSystem -----
CompositorObjectSystem::CompositorObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

std::vector<MikanCompositorID> CompositorObjectSystem::getCompositorIdListForStage(MikanStageID stageId) const
{
	std::vector<MikanCompositorID> compositorIdList;

	for (const auto& compositorPair : Super::getComponentMap())
	{
		CompositorComponentPtr componentPtr = compositorPair.second.lock();

		if (componentPtr && componentPtr->getOwnerStageId() == stageId)
		{
			compositorIdList.push_back(compositorPair.first);
		}
	}

	return compositorIdList;
}

void CompositorObjectSystem::setActiveCompositors(
	const std::vector<MikanCompositorID>& activeCompositorIdList)
{
	// Iterate through all compositor components
	for (const auto& compositorPair : Super::getComponentMap())
	{
		MikanCompositorID compositorId = compositorPair.first;
		CompositorComponentPtr compositor = compositorPair.second.lock();

		if (compositor)
		{
			// Check if this compositor should be active
			bool shouldBeActive = std::find(activeCompositorIdList.begin(), activeCompositorIdList.end(), compositorId) != activeCompositorIdList.end();
			bool isCurrentlyRunning = compositor->getIsRunning();

			if (shouldBeActive && !isCurrentlyRunning)
			{
				// Start the compositor if it should be active but isn't running
				if (compositor->start())
				{
					OnCompositorActivated(compositor);
				}
			}
			else if (!shouldBeActive && isCurrentlyRunning)
			{
				// Stop the compositor if it shouldn't be active but is running
				compositor->stop();
				OnCompositorDeactivated(compositor);
			}
		}
	}
}