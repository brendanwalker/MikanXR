#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanClientTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class SceneComponent;
using SceneComponentWeakPtr = std::weak_ptr<SceneComponent>;

class FastenerConfig : public CommonConfig
{
public:
	FastenerConfig();
	FastenerConfig(
		MikanSpatialFastenerID fastenerId,
		const std::string& fastenerName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanSpatialFastenerID getFastenerId() const { return m_fastenerInfo.fastener_id; }
	MikanFastenerParentType getFastenerParentType() const { return m_fastenerInfo.parent_object_type; }
	int32_t getParentObjectId() const { return m_fastenerInfo.parent_object_id; }
	bool getParentAnchorId(MikanSpatialAnchorID& outAnchorId) const;
	bool getParentStencilId(MikanStencilID& outStencilId) const;
	const MikanSpatialFastenerInfo& getFastenerInfo() const { return m_fastenerInfo; }

	const std::string getFastenerName() const;
	void setFastenerName(const std::string& fastenerName);

	void getFastenerLocalPoints(glm::vec3 outLocalPoints[3]) const;

private:
	MikanSpatialFastenerInfo m_fastenerInfo;
};

class FastenerComponent : public MikanComponent
{
public:
	FastenerComponent(MikanObjectWeakPtr owner);

	inline FastenerConfigPtr getConfig() const { return m_config; }
	void setConfig(FastenerConfigPtr config);

	const std::string getFastenerName();
	void setFastenerName(const std::string& newFastenerName);

	glm::mat4 getFastenerWorldTransform() const;

	void getFastenerLocalPoints(glm::vec3 outLocalPoints[3]) const;
	void getFastenerWorldPoints(glm::vec3 outWorldPoints[3]) const;

protected:
	FastenerConfigPtr m_config;
};

extern const std::string* k_fastenerParentTypeStrings;