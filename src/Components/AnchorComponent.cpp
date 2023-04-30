#include "AnchorComponent.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "SceneComponent.h"
#include "MikanObject.h"
#include "MikanServer.h"
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

	// Tell any connected clients that the anchor pose changed
	{
		MikanAnchorPoseUpdateEvent poseUpdateEvent;
		memset(&poseUpdateEvent, 0, sizeof(MikanAnchorPoseUpdateEvent));
		poseUpdateEvent.anchor_id = m_anchorInfo.anchor_id;
		poseUpdateEvent.transform = m_anchorInfo.anchor_xform;

		MikanServer::getInstance()->publishAnchorPoseUpdatedEvent(poseUpdateEvent);
	}
}

const std::string AnchorConfig::getAnchorName() const
{
	return m_anchorInfo.anchor_name;
}

void AnchorConfig::setAnchorName(const std::string& anchorName)
{
	strncpy(m_anchorInfo.anchor_name, anchorName.c_str(), sizeof(m_anchorInfo.anchor_name) - 1);
	markDirty();

	// Tell any connected clients that the anchor list changed
	MikanServer::getInstance()->publishAnchorListChangedEvent();
}

// -- AnchorComponent -----
AnchorComponent::AnchorComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{}

void AnchorComponent::init()
{
	MikanComponent::init();

	SceneComponentPtr sceneComponentPtr = getOwnerObject()->getComponentOfType<SceneComponent>();
	sceneComponentPtr->OnTranformChaged += MakeDelegate(this, &AnchorComponent::applySceneComponentTransformToConfig);
	sceneComponentPtr->setGlLineRenderable(getSelfPtr<AnchorComponent>());
	m_sceneComponent= sceneComponentPtr;

	applyConfigTransformToSceneComponent();
}

void AnchorComponent::renderLines() const
{	
	TextStyle style = getDefaultTextStyle();

	wchar_t wszAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
	StringUtils::convertMbsToWcs(m_config->getAnchorName().c_str(), wszAnchorName, sizeof(wszAnchorName));
	glm::mat4 anchorXform = m_config->getAnchorXform();
	glm::vec3 anchorPos(anchorXform[3]);

	drawTransformedAxes(anchorXform, 0.1f, 0.1f, 0.1f);
	drawTextAtWorldPosition(style, anchorPos, L"%s", wszAnchorName);
}

void AnchorComponent::dispose()
{
	SceneComponentPtr sceneComponentPtr = m_sceneComponent.lock();
	sceneComponentPtr->OnTranformChaged -= MakeDelegate(this, &AnchorComponent::applySceneComponentTransformToConfig);
}


void AnchorComponent::setConfig(AnchorConfigPtr config)
{
	m_config= config;
	applyConfigTransformToSceneComponent();
}

glm::mat4 AnchorComponent::getAnchorLocalTransform() const
{
	return m_sceneComponent.lock()->getRelativeTransform().getMat4();
}

glm::mat4 AnchorComponent::getAnchorWorldTransform() const
{
	return m_sceneComponent.lock()->getWorldTransform();
}

void AnchorComponent::setAnchorLocalTransform(const GlmTransform& localTransform)
{
	m_sceneComponent.lock()->setRelativeTransform(localTransform);
}

void AnchorComponent::setAnchorWorldTransform(const glm::mat4& worldMat)
{
	m_sceneComponent.lock()->setWorldTransform(worldMat);
}

const std::string AnchorComponent::getAnchorName()
{
	return m_config->getAnchorName();
}

void AnchorComponent::setAnchorName(const std::string& newAnchorName)
{
	m_config->setAnchorName(newAnchorName);
}

void AnchorComponent::applyConfigTransformToSceneComponent()
{
	SceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		GlmTransform relativeTransform(m_config->getAnchorXform());

		m_bIsApplyingConfigTransform= true;
		sceneComponent->setRelativeTransform(relativeTransform);
		m_bIsApplyingConfigTransform= false;
	}
}

void AnchorComponent::applySceneComponentTransformToConfig(SceneComponentPtr sceneComponent)
{
	// Ignore this call if we were invoked from applyConfigTransformToSceneComponent()
	if (m_bIsApplyingConfigTransform)
		return;

	m_config->setAnchorXform(sceneComponent->getRelativeTransform().getMat4());
}