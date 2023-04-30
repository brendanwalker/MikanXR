#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanClientTypes.h"
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
	FastenerConfigPtr getSpatialFastenerConfig(MikanSpatialFastenerID fastenerId) const;
	FastenerConfigPtr getSpatialFastenerConfigByName(const std::string& fastenerName) const;
	MikanSpatialFastenerID addNewFastener(const MikanSpatialFastenerInfo& fastenerInfo);
	bool removeFastener(MikanSpatialFastenerID fastenerId);

	std::vector<FastenerConfigPtr> spatialFastenerList;
	MikanSpatialFastenerID nextFastenerId;
	MikanSpatialFastenerID originFastenerId;
	bool debugRenderFasteners;
};


class FastenerObjectSystem : public MikanObjectSystem
{
public:
	static FastenerObjectSystemPtr getSystem() { return s_fastenerObjectSystem.lock(); }

	FastenerObjectSystemConfigConstPtr getFastenerSystemConfigConst() const;
	FastenerObjectSystemConfigPtr getFastenerSystemConfig();

	virtual void init() override;
	virtual void dispose() override;

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