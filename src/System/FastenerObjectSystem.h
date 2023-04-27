#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanClientTypes.h"
#include "ObjectSystemConfigFwd.h"

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
	FastenerConfigPtr getSpatialFastenerConfig(MikanSpatialFastenerID FastenerId) const;
	FastenerConfigPtr getSpatialFastenerConfigByName(const std::string& FastenerName) const;
	MikanSpatialFastenerID addNewFastener(const std::string& FastenerName);
	bool removeFastener(MikanSpatialFastenerID FastenerId);

	std::string FastenerVRDevicePath;
	std::vector<FastenerConfigPtr> spatialFastenerList;
	MikanSpatialFastenerID nextFastenerId;
	MikanSpatialFastenerID originFastenerId;
	bool debugRenderFasteners;
};


class FastenerObjectSystem : public MikanObjectSystem
{
public:
	FastenerObjectSystem();
	virtual ~FastenerObjectSystem();

	static FastenerObjectSystem* getSystem() { return s_fastenerObjectSystem; }

	virtual void init() override;
	virtual void dispose() override;

	const FastenerMap& getFastenerMap() const { return m_fastenerComponents; }
	FastenerComponentPtr getSpatialFastenerById(MikanSpatialFastenerID FastenerId) const;
	FastenerComponentPtr getSpatialFastenerByName(const std::string& FastenerName) const;
	std::vector<MikanSpatialFastenerID> getSpatialFastenersWithParent(
		const MikanFastenerParentType parentType,
		const MikanSpatialAnchorID parentObjectId) const;
	std::vector<MikanSpatialFastenerID> getValidSpatialFastenerSnapTargets(
		const MikanSpatialFastenerID sourceFastenerId) const;
	FastenerComponentPtr addNewFastener(const std::string& FastenerName);
	bool removeFastener(MikanSpatialFastenerID FastenerId);

protected:
	FastenerObjectSystemConfigConstPtr getFastenerSystemConfigConst() const;
	FastenerObjectSystemConfigPtr getFastenerSystemConfig();

	FastenerComponentPtr createFastenerObject(FastenerConfigPtr fastenerConfig);
	void disposeFastenerObject(MikanSpatialFastenerID FastenerId);

	FastenerMap m_fastenerComponents;

	static FastenerObjectSystem* s_fastenerObjectSystem;
};