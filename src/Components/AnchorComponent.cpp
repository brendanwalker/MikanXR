#include "AnchorComponent.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "SceneComponent.h"
#include "MikanObject.h"
//#include "MikanServer.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

// -- AnchorConfig -----
const std::string AnchorConfig::k_anchorNamePropertyID= "anchor_name";
const std::string AnchorConfig::k_anchorXformPropertyID= "anchor_xform";

AnchorConfig::AnchorConfig()
	: CommonConfig("")
{
	memset(&m_anchorInfo, 0, sizeof(MikanSpatialAnchorInfo));
	m_anchorInfo.anchor_id = INVALID_MIKAN_ID;
	m_anchorInfo.anchor_xform = glm_mat4_to_MikanMatrix4f(glm::mat4(1.f));
}

AnchorConfig::AnchorConfig(
	MikanSpatialAnchorID anchorId,
	const std::string& anchorName,
	const MikanMatrix4f& xform)
	: CommonConfig(StringUtils::stringify("Anchor_", anchorId))
{
	memset(&m_anchorInfo, 0, sizeof(MikanSpatialAnchorInfo));
	m_anchorInfo.anchor_id = anchorId;
	strncpy(m_anchorInfo.anchor_name, anchorName.c_str(), sizeof(m_anchorInfo.anchor_name) - 1);
	m_anchorInfo.anchor_xform = xform;
}

configuru::Config AnchorConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["id"] = m_anchorInfo.anchor_id;
	pt["name"] = m_anchorInfo.anchor_name;

	writeMatrix4f(pt, "xform", m_anchorInfo.anchor_xform);

	return pt;
}

void AnchorConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_anchorInfo.anchor_id = pt.get<int>("id");
		m_configName = StringUtils::stringify("Anchor_", m_anchorInfo.anchor_id);
	}

	if (pt.has_key("name"))
	{
		std::string anchorName = pt.get<std::string>("name");
		strncpy(m_anchorInfo.anchor_name, anchorName.c_str(), sizeof(m_anchorInfo.anchor_name) - 1);
	}

	if (pt.has_key("xform"))
	{
		readMatrix4f(pt, "xform", m_anchorInfo.anchor_xform);
	}
}

const glm::mat4 AnchorConfig::getAnchorXform() const
{
	return MikanMatrix4f_to_glm_mat4(m_anchorInfo.anchor_xform);
}

void AnchorConfig::setAnchorXform(const glm::mat4& xform)
{
	m_anchorInfo.anchor_xform = glm_mat4_to_MikanMatrix4f(xform);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_anchorXformPropertyID));
}

const std::string AnchorConfig::getAnchorName() const
{
	return m_anchorInfo.anchor_name;
}

void AnchorConfig::setAnchorName(const std::string& anchorName)
{
	strncpy(m_anchorInfo.anchor_name, anchorName.c_str(), sizeof(m_anchorInfo.anchor_name) - 1);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_anchorNamePropertyID));
}

// -- AnchorComponent -----
AnchorComponent::AnchorComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
	m_bWantsCustomRender= true;
}

void AnchorComponent::init()
{
	MikanComponent::init();

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
}

void AnchorComponent::customRender()
{	
	TextStyle style = getDefaultTextStyle();

	wchar_t wszAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
	StringUtils::convertMbsToWcs(m_config->getAnchorName().c_str(), wszAnchorName, sizeof(wszAnchorName));
	glm::mat4 anchorXform = getWorldTransform();
	glm::vec3 anchorPos(anchorXform[3]);

	drawTransformedAxes(anchorXform, 0.1f, 0.1f, 0.1f);
	drawTextAtWorldPosition(style, anchorPos, L"%s", wszAnchorName);
}

void AnchorComponent::setConfig(AnchorConfigPtr config)
{
	assert(!m_bIsInitialized);
	m_config= config;

	// Initially the anchor component isn't attached to anything
	assert(m_parentComponent.lock() == nullptr);
	m_worldTransform= config->getAnchorXform();
	m_relativeTransform= GlmTransform(m_worldTransform);

	// Make the component name match the config name
	m_name= config->getAnchorName();
}

void AnchorComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	SceneComponent::setRelativeTransform(newRelativeXform);

	if (m_bIsInitialized)
	{
		m_config->setAnchorXform(newRelativeXform.getMat4());
	}
}

void AnchorComponent::setWorldTransform(const glm::mat4& newWorldXform)
{
	SceneComponent::setWorldTransform(newWorldXform);

	if (m_bIsInitialized)
	{
		m_config->setAnchorXform(getRelativeTransform().getMat4());
	}
}

void AnchorComponent::setName(const std::string& name)
{
	SceneComponent::setName(name);

	if (m_bIsInitialized)
	{
		m_config->setAnchorName(name);
	}
}