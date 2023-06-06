#include "AnchorComponent.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

// -- AnchorConfig -----
AnchorDefinition::AnchorDefinition()
	: SceneComponentDefinition()
{
	memset(&m_anchorInfo, 0, sizeof(MikanSpatialAnchorInfo));
	m_anchorInfo.anchor_id = INVALID_MIKAN_ID;
	m_anchorInfo.relative_transform.scale = {1.f, 1.f, 1.f};
	m_anchorInfo.relative_transform.rotation= {1.f, 0.0, 0.f, 0.f};
	m_anchorInfo.relative_transform.translation= {0.f, 0.f, 0.f};
}

AnchorDefinition::AnchorDefinition(
	MikanSpatialAnchorID anchorId,
	const std::string& anchorName,
	const MikanTransform& xform)
	: SceneComponentDefinition(StringUtils::stringify("Anchor_", anchorId), xform)
{
	memset(&m_anchorInfo, 0, sizeof(MikanSpatialAnchorInfo));
	m_anchorInfo.anchor_id = anchorId;
	strncpy(m_anchorInfo.anchor_name, anchorName.c_str(), sizeof(m_anchorInfo.anchor_name) - 1);
	m_anchorInfo.relative_transform = xform;
}

configuru::Config AnchorDefinition::writeToJSON()
{
	configuru::Config pt = SceneComponentDefinition::writeToJSON();

	pt["id"] = m_anchorInfo.anchor_id;

	return pt;
}

void AnchorDefinition::readFromJSON(const configuru::Config& pt)
{
	SceneComponentDefinition::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_anchorInfo.anchor_id = pt.get<int>("id");
		m_configName = StringUtils::stringify("Anchor_", m_anchorInfo.anchor_id);
	}
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

	// Watch selection changes
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
}

void AnchorComponent::customRender()
{	
	TextStyle style = getDefaultTextStyle();

	wchar_t wszAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
	StringUtils::convertMbsToWcs(m_definition->getComponentName().c_str(), wszAnchorName, sizeof(wszAnchorName));
	glm::mat4 anchorXform = getWorldTransform();
	glm::vec3 anchorPos(anchorXform[3]);

	glm::vec3 xColor = Colors::DarkRed;
	glm::vec3 yColor = Colors::DarkGreen;
	glm::vec3 zColor = Colors::DarkBlue;
	SelectionComponentPtr selectionComponent = m_selectionComponent.lock();
	if (selectionComponent)
	{
		if (selectionComponent->getIsSelected())
		{
			xColor = Colors::Red;
			yColor = Colors::Green;
			zColor = Colors::Blue;
		}
		else if (selectionComponent->getIsHovered())
		{
			xColor = Colors::LightGreen;
			yColor = Colors::LightGreen;
			zColor = Colors::LightBlue;
		}
	}

	drawTransformedAxes(anchorXform, 0.1f, 0.1f, 0.1f, xColor, yColor, zColor);
	drawTextAtWorldPosition(style, anchorPos, L"%s", wszAnchorName);
}