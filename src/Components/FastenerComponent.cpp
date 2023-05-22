#include "AnchorObjectSystem.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "FastenerComponent.h"
#include "FastenerObjectSystem.h"
#include "SceneComponent.h"
#include "StencilComponent.h"
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
const std::string FastenerConfig::kFastenerNamePropertyId= "fastener_name";
const std::string FastenerConfig::kFastenerPointsPropertyId= "fastener_points";

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
	else if (m_fastenerInfo.parent_object_type == MikanFastenerParentType_Stencil)
	{
		StencilComponentPtr stencilPtr= 
			StencilObjectSystem::getSystem()->getStencilById(m_fastenerInfo.parent_object_id);

		if (stencilPtr != nullptr)
		{
			outAnchorId= stencilPtr->getParentAnchorId();
			return true;
		}
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
	markDirty(ConfigPropertyChangeSet().addPropertyName(kFastenerNamePropertyId));
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
	markDirty(ConfigPropertyChangeSet().addPropertyName(kFastenerPointsPropertyId));
}

// -- FastenerComponent -----
FastenerComponent::FastenerComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
	m_bWantsCustomRender= true;
}

void FastenerComponent::init()
{
	MikanComponent::init();

	SceneComponentPtr sceneComponentPtr = getOwnerObject()->getRootComponent();
}

void FastenerComponent::customRender()
{
	TextStyle style = getDefaultTextStyle();

	const MikanSpatialFastenerInfo& fastener = m_config->getFastenerInfo();
	wchar_t wszFastenerName[MAX_MIKAN_FASTENER_NAME_LEN];
	StringUtils::convertMbsToWcs(fastener.fastener_name, wszFastenerName, sizeof(wszFastenerName));

	const glm::mat4 xform = getWorldTransform();
	const glm::vec3 p0 = MikanVector3f_to_glm_vec3(fastener.fastener_points[0]);
	const glm::vec3 p1 = MikanVector3f_to_glm_vec3(fastener.fastener_points[1]);
	const glm::vec3 p2 = MikanVector3f_to_glm_vec3(fastener.fastener_points[2]);

	drawArrow(xform, p0, p1, 0.01f, Colors::Red);
	drawArrow(xform, p0, p2, 0.01f, Colors::Green);

	const glm::vec3 text_pos = xform * glm::vec4(p0, 1.f);
	drawTextAtWorldPosition(style, text_pos, L"%s", wszFastenerName);
}

void FastenerComponent::setConfig(FastenerConfigPtr config)
{
	assert(!m_bIsInitialized);
	m_config = config;

	// Initially the fastener component isn't attached to anything
	assert(m_parentComponent.lock() == nullptr);

	// Make the component name match the config name
	m_name= m_config->getFastenerName();
}

const std::string FastenerComponent::getFastenerName()
{
	return m_config->getFastenerName();
}

void FastenerComponent::setFastenerName(const std::string& newFastenerName)
{
	m_config->setFastenerName(newFastenerName);
	setName(newFastenerName);
}

void FastenerComponent::getFastenerLocalPoints(glm::vec3 outLocalPoints[3]) const
{
	return m_config->getFastenerLocalPoints(outLocalPoints);
}

void FastenerComponent::getFastenerWorldPoints(glm::vec3 outWorldPoints[3]) const
{
	const glm::mat4 localToWorld = getWorldTransform();

	glm::vec3 localPoints[3];
	getFastenerLocalPoints(localPoints);
	for (int i = 0; i < 3; ++i)
	{
		outWorldPoints[i] = localToWorld * glm::vec4(localPoints[i], 1.f);
	}
}

