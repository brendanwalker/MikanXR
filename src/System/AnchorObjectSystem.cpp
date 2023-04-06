#include "App.h"
#include "AnchorObjectSystem.h"
#include "MikanAnchorComponent.h"
#include "MikanSceneComponent.h"
#include "MathTypeConversion.h"
#include "ProfileConfig.h"

AnchorObjectSystem* AnchorObjectSystem::s_anchorObjectSystem= nullptr;

AnchorObjectSystem::AnchorObjectSystem()
{
	s_anchorObjectSystem = this;
}

AnchorObjectSystem::~AnchorObjectSystem()
{
	s_anchorObjectSystem= nullptr;
}

void AnchorObjectSystem::init()
{
	MikanObjectSystem::init();

	const AnchorObjectSystemConfig& anchorConfig = getAnchorConfigConst();
	for (const MikanSpatialAnchorInfo& anchorInfo : anchorConfig.spatialAnchorList)
	{
		createAnchorObject(anchorInfo);
	}
}

void AnchorObjectSystem::dispose()
{
	m_anchorComponents.clear();
	MikanObjectSystem::dispose();
}

MikanAnchorComponentWeakPtr AnchorObjectSystem::getSpatialAnchorById(MikanSpatialAnchorID anchorId) const
{
	auto iter= m_anchorComponents.find(anchorId);
	if (iter != m_anchorComponents.end())
	{
		return iter->second;
	}

	return MikanAnchorComponentWeakPtr();
}

MikanAnchorComponentWeakPtr AnchorObjectSystem::getSpatialAnchorByName(const std::string& anchorName) const
{
	for (auto it = m_anchorComponents.begin(); it != m_anchorComponents.end(); it++)
	{
		MikanAnchorComponentPtr componentPtr= it->second.lock();

		if (componentPtr && componentPtr->getAnchorName() == anchorName)
		{
			return componentPtr;
		}
	}

	return MikanAnchorComponentWeakPtr();
}

MikanAnchorComponentPtr AnchorObjectSystem::addNewAnchor(const std::string& anchorName, const glm::mat4& xform)
{
	AnchorObjectSystemConfig& anchorConfig = getAnchorConfig();

	MikanSpatialAnchorID anchorId= anchorConfig.addNewAnchor(anchorName, glm_mat4_to_MikanMatrix4f(xform));
	if (anchorId != INVALID_MIKAN_ID)
	{
		const MikanSpatialAnchorInfo* anchorInfo= anchorConfig.getSpatialAnchorInfo(anchorId);
		assert(anchorInfo != nullptr);
	}

	return MikanAnchorComponentPtr();
}

bool AnchorObjectSystem::removeAnchor(MikanSpatialAnchorID anchorId)
{
	getAnchorConfig().removeAnchor(anchorId);
	disposeAnchorObject(anchorId);

	return false;
}

MikanAnchorComponentPtr AnchorObjectSystem::createAnchorObject(const MikanSpatialAnchorInfo& anchorInfo)
{
	MikanObjectPtr anchorObject= newObject();

	// Add a scene component to the anchor
	MikanSceneComponentPtr sceneComponentPtr= anchorObject->addComponent<MikanSceneComponent>();
	anchorObject->setRootComponent(sceneComponentPtr);

	// Add spatial anchor component to the object
	MikanAnchorComponentPtr anchorComponentPtr= anchorObject->addComponent<MikanAnchorComponent>();
	anchorComponentPtr->setSpatialAnchor(anchorInfo);
	m_anchorComponents.insert({anchorInfo.anchor_id, anchorComponentPtr});

	// Init the object once all components are added
	anchorObject->init();

	return anchorComponentPtr;
}

void AnchorObjectSystem::disposeAnchorObject(MikanSpatialAnchorID anchorId)
{
	auto it= m_anchorComponents.find(anchorId);
	if (it != m_anchorComponents.end())
	{
		MikanAnchorComponentPtr anchorComponentPtr= it->second.lock();

		// Remove for component list
		m_anchorComponents.erase(it);

		// Free the corresponding object
		removeObject(anchorComponentPtr->getOwnerObject());
	}
}

const AnchorObjectSystemConfig& AnchorObjectSystem::getAnchorConfigConst() const
{
	return App::getInstance()->getProfileConfig()->anchorConfig;
}

AnchorObjectSystemConfig& AnchorObjectSystem::getAnchorConfig()
{
	return const_cast<AnchorObjectSystemConfig&>(getAnchorConfigConst());
}