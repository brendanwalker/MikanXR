#include "App.h"
#include "MikanObject.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "ProjectConfig.h"

// -- CompositorObjectSystemConfig -----
const std::string CompositorObjectSystemConfig::k_compositorListPropertyId= "compositorList";

configuru::Config CompositorObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["next_compositor_id"] = m_nextCompositorId;

	std::vector<configuru::Config> compositorListConfigs;
	for (auto definitionPtr : m_compositorList)
	{
		compositorListConfigs.push_back(definitionPtr->writeToJSON());
	}
	pt.insert_or_assign(std::string("compositors"), compositorListConfigs);

	return pt;
}

void CompositorObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextCompositorId = pt.get_or<int>("next_compositor_id", m_nextCompositorId);

	// Read in the compositor definitions
	m_compositorList.clear();
	if (pt.has_key("compositors"))
	{
		for (const configuru::Config& compositor_pt : pt["compositors"].as_array())
		{
			auto definitionPtr = std::make_shared<CompositorDefinition>();

			definitionPtr->readFromJSON(compositor_pt);
			m_compositorList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

CompositorDefinitionPtr CompositorObjectSystemConfig::getCompositorConfig(MikanCompositorID compositorId) const
{
	auto it = std::find_if(
		m_compositorList.begin(), m_compositorList.end(),
		[compositorId](CompositorDefinitionPtr configPtr) {
			return configPtr->getCompositorId() == compositorId;
		});

	if (it != m_compositorList.end())
	{
		return CompositorDefinitionPtr(*it);
	}

	return CompositorDefinitionPtr();
}

CompositorDefinitionPtr CompositorObjectSystemConfig::getCompositorConfigByName(const std::string& compositorName) const
{
	auto it = std::find_if(
		m_compositorList.begin(), m_compositorList.end(),
		[compositorName](CompositorDefinitionPtr configPtr) {
			return configPtr->getComponentName() == compositorName;
		});

	if (it != m_compositorList.end())
	{
		return CompositorDefinitionPtr(*it);
	}

	return CompositorDefinitionPtr();
}

MikanCompositorID CompositorObjectSystemConfig::addNewCompositor(
	MikanStageID ownerStageId)
{
	std::string compositorName = "Compositor_" + std::to_string(m_nextCompositorId);
	auto compositorDefinitionPtr = 
		std::make_shared<CompositorDefinition>(
			m_nextCompositorId, ownerStageId, compositorName);
	m_nextCompositorId++;

	m_compositorList.push_back(compositorDefinitionPtr);
	addChildConfig(compositorDefinitionPtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_compositorListPropertyId));

	return compositorDefinitionPtr->getCompositorId();
}

bool CompositorObjectSystemConfig::removeCompositor(MikanCompositorID compositorId)
{
	auto it = std::find_if(
		m_compositorList.begin(), m_compositorList.end(),
		[compositorId](CompositorDefinitionPtr configPtr) {
			return configPtr->getCompositorId() == compositorId;
		});

	if (it != m_compositorList.end())
	{
		removeChildConfig(*it);

		m_compositorList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_compositorListPropertyId));

		return true;
	}

	return false;
}

// -- CompositorObjectSystem -----
CompositorObjectSystemWeakPtr CompositorObjectSystem::s_compositorObjectSystem;

bool CompositorObjectSystem::init()
{
	MikanObjectSystem::init();

	CompositorObjectSystemConfigConstPtr compositorSystemConfig = getCompositorSystemConfigConst();

	s_compositorObjectSystem = std::static_pointer_cast<CompositorObjectSystem>(shared_from_this());
	return true;
}

void CompositorObjectSystem::dispose()
{
	s_compositorObjectSystem.reset();

	MikanObjectSystem::dispose();
}

CompositorObjectSystemConfigConstPtr CompositorObjectSystem::getCompositorSystemConfigConst() const
{
	return getProjectConfig()->compositorConfig;
}

CompositorObjectSystemConfigPtr CompositorObjectSystem::getCompositorSystemConfig()
{
	return std::const_pointer_cast<CompositorObjectSystemConfig>(getCompositorSystemConfigConst());
}

CompositorComponentPtr CompositorObjectSystem::getCompositorById(MikanCompositorID compositorId) const
{
	auto iter = m_compositorComponents.find(compositorId);
	if (iter != m_compositorComponents.end())
	{
		return iter->second.lock();
	}

	return CompositorComponentPtr();
}

CompositorComponentPtr CompositorObjectSystem::getCompositorByName(const std::string& compositorName) const
{
	for (auto it = m_compositorComponents.begin(); it != m_compositorComponents.end(); it++)
	{
		CompositorComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getName() == compositorName)
		{
			return componentPtr;
		}
	}

	return CompositorComponentPtr();
}

std::vector<MikanCompositorID> CompositorObjectSystem::getCompositorIdListForStage(MikanStageID stageId) const
{
	std::vector<MikanCompositorID> compositorIdList;

	for (auto it = m_compositorComponents.begin(); it != m_compositorComponents.end(); it++)
	{
		CompositorComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getOwnerStageId() == stageId)
		{
			compositorIdList.push_back(it->first);
		}
	}

	return compositorIdList;
}

CompositorComponentPtr CompositorObjectSystem::addNewCompositor(
	MikanStageID ownerStageId)
{
	CompositorObjectSystemConfigPtr compositorSystemConfig = getCompositorSystemConfig();

	if (compositorSystemConfig)
	{
		MikanCompositorID compositorId = compositorSystemConfig->addNewCompositor(ownerStageId);
		if (compositorId != INVALID_MIKAN_ID)
		{
			CompositorDefinitionPtr compositorConfig = compositorSystemConfig->getCompositorConfig(compositorId);
			assert(compositorConfig != nullptr);

			return createCompositorObject(compositorConfig);
		}
	}

	return CompositorComponentPtr();
}

bool CompositorObjectSystem::removeCompositor(MikanCompositorID compositorId)
{
	bool bValidCompositor = false;
	CompositorObjectSystemConfigPtr compositorSystemConfig = getCompositorSystemConfig();
	
	if (compositorSystemConfig)
	{
		bValidCompositor = compositorSystemConfig->removeCompositor(compositorId);
	}
	
	disposeCompositorObject(compositorId);

	return bValidCompositor;
}

void CompositorObjectSystem::setActiveCompositors(
	const std::vector<MikanCompositorID>& activeCompositorIdList)
{
	// Iterate through all compositor components
	for (const auto& compositorPair : m_compositorComponents)
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

CompositorComponentPtr CompositorObjectSystem::createCompositorObject(CompositorDefinitionPtr compositorConfig)
{
	CompositorObjectSystemConfigConstPtr compositorSystemConfig = getCompositorSystemConfigConst();
	MikanObjectPtr compositorObject = newObject();
	compositorObject->setName(compositorConfig->getComponentName());

	// Add compositor component to the object
	CompositorComponentPtr compositorComponentPtr = compositorObject->addComponent<CompositorComponent>();
	compositorComponentPtr->setDefinition(compositorConfig);
	m_compositorComponents.insert({compositorConfig->getCompositorId(), compositorComponentPtr});

	// Init the object once all components are added
	compositorObject->init();

	return compositorComponentPtr;
}

void CompositorObjectSystem::disposeCompositorObject(MikanCompositorID compositorId)
{
	auto it = m_compositorComponents.find(compositorId);
	if (it != m_compositorComponents.end())
	{
		CompositorComponentPtr pendingDisposeCompositor = it->second.lock();

		// Remove for component list
		m_compositorComponents.erase(it);

		// Free the corresponding object
		deleteObject(pendingDisposeCompositor->getOwnerObject());
	}
}