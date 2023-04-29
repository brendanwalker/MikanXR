#include "AnchorObjectSystem.h"
#include "FastenerComponent.h"
#include "SceneComponent.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"

const std::string g_fastenerParentTypeStrings[MikanFastenerParentType_COUNT] = {
	"SpatialAnchor",
	"Stencil"
};
extern const std::string* k_fastenerParentTypeStrings = g_fastenerParentTypeStrings;

// -- FastenerConfig -----
FastenerConfig::FastenerConfig()
	: CommonConfig("")
{
	memset(&m_fastenerInfo, 0, sizeof(MikanSpatialFastenerInfo));
	m_fastenerInfo.fastener_id = INVALID_MIKAN_ID;
}

FastenerConfig::FastenerConfig(
	MikanSpatialFastenerID fastenerId,
	const MikanSpatialFastenerInfo& fastenerInfo)
	: CommonConfig(StringUtils::stringify("fastener_", fastenerId))
{
	m_fastenerInfo= fastenerInfo;
	m_fastenerInfo.fastener_id = fastenerId;
}

configuru::Config FastenerConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["id"] = m_fastenerInfo.fastener_id;
	pt["parent_object_id"] = m_fastenerInfo.parent_object_id;
	pt["parent_object_type"] = k_fastenerParentTypeStrings[m_fastenerInfo.parent_object_type];
	pt["name"] = m_fastenerInfo.fastener_name;

	writeVector3f(pt, "point0", m_fastenerInfo.fastener_points[0]);
	writeVector3f(pt, "point1", m_fastenerInfo.fastener_points[1]);
	writeVector3f(pt, "point2", m_fastenerInfo.fastener_points[2]);

	return pt;
}

void FastenerConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	memset(&m_fastenerInfo, 0, sizeof(m_fastenerInfo));

	if (pt.has_key("id"))
	{
		m_fastenerInfo.fastener_id = pt.get<int>("id");
		m_configName = StringUtils::stringify("fastener_", m_fastenerInfo.fastener_id);
	}

	if (pt.has_key("name"))
	{
		std::string FastenerName = pt.get<std::string>("name");
		strncpy(m_fastenerInfo.fastener_name, FastenerName.c_str(), sizeof(m_fastenerInfo.fastener_name) - 1);
	}

	if (pt.has_key("parent_object_id"))
	{
		m_fastenerInfo.parent_object_id = pt.get<int>("parent_object_id");
	}

	if (pt.has_key("parent_object_type"))
	{
		// Parse the parent object type enum
		const std::string parentTypeString =
			pt.get_or<std::string>(
				"parent_object_type",
				k_fastenerParentTypeStrings[MikanFastenerParentType_SpatialAnchor]);

		m_fastenerInfo.parent_object_type = MikanFastenerParentType_UNKNOWN;
		for (size_t enum_index = 0; enum_index < MikanFastenerParentType_COUNT; ++enum_index)
		{
			if (parentTypeString == k_fastenerParentTypeStrings[enum_index])
			{
				m_fastenerInfo.parent_object_type = (MikanFastenerParentType)enum_index;
				break;
			}
		}
	}

	if (pt.has_key("point0") &&
		pt.has_key("point1") &&
		pt.has_key("point2"))
	{
		readVector3f(pt, "point0", m_fastenerInfo.fastener_points[0]);
		readVector3f(pt, "point1", m_fastenerInfo.fastener_points[1]);
		readVector3f(pt, "point2", m_fastenerInfo.fastener_points[2]);
	}
}

bool FastenerConfig::getParentAnchorId(MikanSpatialAnchorID& outAnchorId) const
{
	if (m_fastenerInfo.parent_object_type == MikanFastenerParentType_SpatialAnchor)
	{
		outAnchorId= m_fastenerInfo.parent_object_id;
		return true;
	}

	return false;
}

bool FastenerConfig::getParentStencilId(MikanStencilID& outStencilId) const
{
	if (m_fastenerInfo.parent_object_type == MikanFastenerParentType_Stencil)
	{
		outStencilId = m_fastenerInfo.parent_object_id;
		return true;
	}

	return false;
}

const std::string FastenerConfig::getFastenerName() const
{
	return m_fastenerInfo.fastener_name;
}

void FastenerConfig::setFastenerName(const std::string& fastenerName)
{
	strncpy(m_fastenerInfo.fastener_name, fastenerName.c_str(), sizeof(m_fastenerInfo.fastener_name) - 1);
	markDirty();
}

void FastenerConfig::getFastenerLocalPoints(glm::vec3 outLocalPoints[3]) const
{
	for (int i = 0; i < 3; ++i)
	{
		outLocalPoints[i] = MikanVector3f_to_glm_vec3(m_fastenerInfo.fastener_points[i]);
	}
}

void FastenerConfig::setFastenerLocalPoints(MikanVector3f inLocalPoints[3])
{
	for (int i = 0; i < 3; ++i)
	{
		m_fastenerInfo.fastener_points[i]= inLocalPoints[i];
	}
	markDirty();
}

// -- FastenerComponent -----
FastenerComponent::FastenerComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{}

void FastenerComponent::setConfig(FastenerConfigPtr config)
{
	m_config = config;
}

const std::string FastenerComponent::getFastenerName()
{
	return m_config->getFastenerName();
}

void FastenerComponent::setFastenerName(const std::string& newFastenerName)
{
	m_config->setFastenerName(newFastenerName);
}

glm::mat4 FastenerComponent::getFastenerWorldTransform() const
{
	return getOwnerObject()->getRootComponent()->getWorldTransform();
}

void FastenerComponent::getFastenerLocalPoints(glm::vec3 outLocalPoints[3]) const
{
	return m_config->getFastenerLocalPoints(outLocalPoints);
}

void FastenerComponent::getFastenerWorldPoints(glm::vec3 outWorldPoints[3]) const
{
	const glm::mat4 localToWorld = getFastenerWorldTransform();

	glm::vec3 localPoints[3];
	getFastenerLocalPoints(localPoints);
	for (int i = 0; i < 3; ++i)
	{
		outWorldPoints[i] = localToWorld * glm::vec4(localPoints[i], 1.f);
	}
}

