#include "AnchorObjectSystem.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "BoxColliderComponent.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "QuadStencilComponent.h"
#include "SelectionComponent.h"
#include "StencilObjectSystemConfig.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"
#include "TextStyle.h"

#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4_precision.hpp"

// -- QuadConfig -----
const std::string QuadStencilDefinition::k_quadStencilWidthPropertyId = "quad_width";
const std::string QuadStencilDefinition::k_quadStencilHeightPropertyId = "quad_height";
const std::string QuadStencilDefinition::k_quadStencilDoubleSidedPropertyId = "is_double_sided";

QuadStencilDefinition::QuadStencilDefinition()
	: StencilComponentDefinition()
	, m_quadWidth(0.f)
	, m_quadHeight(0.f)
	, m_bIsDoubleSided(false)
{
}

QuadStencilDefinition::QuadStencilDefinition(const MikanStencilQuad& quadInfo)
	: StencilComponentDefinition(
		quadInfo.stencil_id,
		quadInfo.parent_anchor_id,
		quadInfo.stencil_name,
		quadInfo.relative_transform)
{
	m_quadWidth= quadInfo.quad_width;
	m_quadHeight= quadInfo.quad_height;
	m_bIsDoubleSided= quadInfo.is_double_sided;
}

configuru::Config QuadStencilDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["quad_width"] = m_quadWidth;
	pt["quad_height"] = m_quadHeight;
	pt["is_double_sided"] = m_bIsDoubleSided;

	return pt;
}

void QuadStencilDefinition::readFromJSON(const configuru::Config& pt)
{
	StencilComponentDefinition::readFromJSON(pt);

	m_quadWidth = pt.get_or<float>("quad_width", 0.25f);
	m_quadHeight = pt.get_or<float>("quad_height", 0.25f);
	m_bIsDoubleSided = pt.get_or<bool>("is_double_sided", false);
}

void QuadStencilDefinition::setQuadWidth(float width)
{
	m_quadWidth = width;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilWidthPropertyId));
}

void QuadStencilDefinition::setQuadHeight(float height)
{
	m_quadHeight = height;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilHeightPropertyId));
}

void QuadStencilDefinition::setQuadSize(float width, float height)
{
	m_quadWidth = width;
	m_quadHeight = height;
	markDirty(ConfigPropertyChangeSet()
				.addPropertyName(k_quadStencilWidthPropertyId)
				.addPropertyName(k_quadStencilHeightPropertyId));
}

void QuadStencilDefinition::setIsDoubleSided(bool flag)
{
	m_bIsDoubleSided = flag;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilDoubleSidedPropertyId));
}

// -- QuadStencilComponent -----
QuadStencilComponent::QuadStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{
	m_bWantsCustomRender= true;
}

void QuadStencilComponent::init()
{
	StencilComponent::init();

	m_boxCollider = getOwnerObject()->getComponentOfType<BoxColliderComponent>();
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	updateBoxColliderExtents();

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
}

void QuadStencilComponent::customRender()
{
	if (!m_definition->getIsDisabled())
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = getWorldTransform();
		const glm::vec3 position = glm::vec3(xform[3]);

		glm::vec3 color = Colors::DarkGray;
		SelectionComponentPtr selectionComponent = m_selectionComponent.lock();
		if (selectionComponent)
		{
			if (selectionComponent->getIsSelected())
				color = Colors::Yellow;
			else if (selectionComponent->getIsHovered())
				color = Colors::LightGray;
		}

		drawTransformedQuad(xform, m_definition->getQuadWidth(), m_definition->getQuadHeight(), color);
		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", m_definition->getStencilId());
	}
}

void QuadStencilComponent::updateBoxColliderExtents()
{
	BoxColliderComponentPtr boxCollider = m_boxCollider.lock();
	if (boxCollider)
	{
		boxCollider->setHalfExtents(glm::vec3(m_definition->getQuadWidth(), m_definition->getQuadHeight(), 0.01f) * 0.5f);
	}
}