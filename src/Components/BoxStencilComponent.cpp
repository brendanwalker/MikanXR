#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "BoxColliderComponent.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "SelectionComponent.h"
#include "StencilObjectSystem.h"
#include "StencilObjectSystemConfig.h"
#include "StringUtils.h"
#include "TextStyle.h"

// -- BoxStencilComponent -----
const std::string BoxStencilDefinition::k_boxStencilXSizePropertyId = "box_x_size";
const std::string BoxStencilDefinition::k_boxStencilYSizePropertyId = "box_y_size";
const std::string BoxStencilDefinition::k_boxStencilZSizePropertyId = "box_z_size";

BoxStencilDefinition::BoxStencilDefinition()
	: StencilComponentDefinition()
	, m_boxSize({0.f, 0.f, 0.f})
{
}

BoxStencilDefinition::BoxStencilDefinition(
	const MikanStencilBox& boxInfo)
	: StencilComponentDefinition(
		boxInfo.stencil_id,
		boxInfo.parent_anchor_id,
		boxInfo.stencil_name,
		boxInfo.relative_transform)
{
	m_boxSize= {boxInfo.box_x_size, boxInfo.box_y_size, boxInfo.box_z_size};
}

configuru::Config BoxStencilDefinition::writeToJSON()
{
	configuru::Config pt = StencilComponentDefinition::writeToJSON();

	pt["box_x_size"] = m_boxSize.x;
	pt["box_y_size"] = m_boxSize.y;
	pt["box_z_size"] = m_boxSize.z;

	return pt;
}

void BoxStencilDefinition::readFromJSON(const configuru::Config& pt)
{
	StencilComponentDefinition::readFromJSON(pt);

	m_boxSize.x = pt.get_or<float>("box_x_size", 0.25f);
	m_boxSize.y = pt.get_or<float>("box_y_size", 0.25f);
	m_boxSize.z = pt.get_or<float>("box_z_size", 0.25f);
}

void BoxStencilDefinition::setBoxXSize(float size)
{
	m_boxSize.x = size;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilXSizePropertyId));
}

void BoxStencilDefinition::setBoxYSize(float size)
{
	m_boxSize.y = size;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilYSizePropertyId));
}

void BoxStencilDefinition::setBoxZSize(float size)
{
	m_boxSize.z = size;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilZSizePropertyId));
}

void BoxStencilDefinition::setBoxSize(float xSize, float ySize, float zSize)
{
	m_boxSize.x = xSize;
	m_boxSize.y = ySize;
	m_boxSize.z = zSize;
	markDirty(ConfigPropertyChangeSet()
				.addPropertyName(k_boxStencilXSizePropertyId)
				.addPropertyName(k_boxStencilYSizePropertyId)
				.addPropertyName(k_boxStencilZSizePropertyId));
}

// -- BoxStencilComponent -----
BoxStencilComponent::BoxStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{
	m_bWantsCustomRender= true;
}

void BoxStencilComponent::init()
{
	StencilComponent::init();

	m_boxCollider = getOwnerObject()->getComponentOfType<BoxColliderComponent>();
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();
}

void BoxStencilComponent::customRender()
{
	if (!m_definition->getIsDisabled())
	{
		TextStyle style = getDefaultTextStyle();

		const float xSize= m_definition->getBoxXSize();
		const float ySize= m_definition->getBoxYSize();
		const float zSize= m_definition->getBoxZSize();
		const glm::mat4 xform = getWorldTransform();
		const glm::vec3 half_extents(xSize / 2.f, ySize / 2.f, zSize / 2.f);
		const glm::vec3 position = glm::vec3(xform[3]);

		glm::vec3 color= Colors::DarkGray;
		SelectionComponentPtr selectionComponent= m_selectionComponent.lock();
		if (selectionComponent)
		{
			if (selectionComponent->getIsSelected())
				color= Colors::Yellow;
			else if (selectionComponent->getIsHovered())
				color= Colors::LightGray;
		}
		
		drawTransformedBox(xform, half_extents, color);
		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", m_definition->getStencilId());
	}
}

void BoxStencilComponent::updateBoxColliderExtents()
{
	BoxColliderComponentPtr boxCollider= m_boxCollider.lock();
	if (boxCollider && m_definition)
	{
		const float xSize = m_definition->getBoxXSize();
		const float ySize = m_definition->getBoxYSize();
		const float zSize = m_definition->getBoxZSize();

		boxCollider->setHalfExtents(glm::vec3(xSize, ySize, zSize) * 0.5f);
	}
}