#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

// -- AnchorConfig -----
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
	markDirty();
}

const std::string AnchorConfig::getAnchorName() const
{
	return m_anchorInfo.anchor_name;
}

void AnchorConfig::setAnchorName(const std::string& anchorName)
{
	strncpy(m_anchorInfo.anchor_name, anchorName.c_str(), sizeof(m_anchorInfo.anchor_name) - 1);
	markDirty();
}

// -- AnchorComponent -----
AnchorComponent::AnchorComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{}

void AnchorComponent::init()
{
	MikanComponent::init();

	m_sceneComponent = getOwnerObject()->getComponentOfType<SceneComponent>();
	updateSceneComponentTransform();
}

void AnchorComponent::setConfig(AnchorConfigPtr config)
{
	m_config= config;
	updateSceneComponentTransform();
}

const glm::mat4 AnchorComponent::getAnchorXform() const
{
	return m_config->getAnchorXform();
}

void AnchorComponent::setAnchorXform(const glm::mat4& xform)
{
	m_config->setAnchorXform(xform);
	updateSceneComponentTransform();
}

const std::string AnchorComponent::getAnchorName()
{
	return m_config->getAnchorName();
}

void AnchorComponent::setAnchorName(const std::string& newAnchorName)
{
	m_config->setAnchorName(newAnchorName);
}

void AnchorComponent::updateSceneComponentTransform()
{
	SceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		sceneComponent->setWorldTransform(m_config->getAnchorXform());
	}
}