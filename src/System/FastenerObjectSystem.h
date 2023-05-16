#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanClientTypes.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "SceneFwd.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

using FastenerMap = std::map<MikanSpatialFastenerID, FastenerComponentWeakPtr>;

class FastenerObjectSystemConfig : public CommonConfig
{
public:
	FastenerObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	bool canAddFastener() const;
	FastenerConfigConstPtr getSpatialFastenerConfig(MikanSpatialFastenerID fastenerId) const;
	FastenerConfigPtr getSpatialFastenerConfig(MikanSpatialFastenerID fastenerId);
	FastenerConfigConstPtr getSpatialFastenerConfigByName(const std::string& fastenerName) const;
	MikanSpatialFastenerID addNewFastener(const MikanSpatialFastenerInfo& fastenerInfo);
	bool removeFastener(MikanSpatialFastenerID fastenerId);
	MulticastDelegate<void()> OnFastenerListChanged;
	MulticastDelegate<void(MikanSpatialFastenerID fastenerId)> OnFastenerModified;

	std::vector<FastenerConfigPtr> spatialFastenerList;
	MikanSpatialFastenerID nextFastenerId= 0;
	MikanSpatialFastenerID originFastenerId= INVALID_MIKAN_ID;
	bool debugRenderFasteners= false;
};


class FastenerObjectSystem : public MikanObjectSystem
{
public:
	static FastenerObjectSystemPtr getSystem() { return s_fastenerObjectSystem.lock(); }

	FastenerObjectSystemConfigConstPtr getFastenerSystemConfigConst() const;
	FastenerObjectSystemConfigPtr getFastenerSystemConfig();

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	const FastenerMap& getFastenerMap() const { return m_fastenerComponents; }
	FastenerComponentPtr getSpatialFastenerById(MikanSpatialFastenerID FastenerId) const;
	FastenerComponentPtr getSpatialFastenerByName(const std::string& FastenerName) const;
	static SceneComponentPtr getFastenerParentSceneComponent(const MikanSpatialFastenerInfo& fastenerInfo);
	std::vector<MikanSpatialFastenerID> getSpatialFastenersWithParent(
		const MikanFastenerParentType parentType,
		const MikanSpatialAnchorID parentObjectId) const;
	std::vector<MikanSpatialFastenerID> getValidSpatialFastenerSnapTargets(
		const MikanSpatialFastenerID sourceFastenerId) const;
	FastenerComponentPtr addNewFastener(const MikanSpatialFastenerInfo& fastenerInfo);
	bool removeFastener(MikanSpatialFastenerID FastenerId);

protected:
	FastenerComponentPtr createFastenerObject(FastenerConfigPtr fastenerConfig);
	void disposeFastenerObject(MikanSpatialFastenerID FastenerId);

	FastenerMap m_fastenerComponents;

	static FastenerObjectSystemWeakPtr s_fastenerObjectSystem;
};