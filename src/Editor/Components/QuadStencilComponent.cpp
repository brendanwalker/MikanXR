#include "AnchorObjectSystem.h"
#include "Colors.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "AnchorComponent.h"
#include "TransformComponent.h"
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

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

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

QuadStencilDefinition::QuadStencilDefinition(const MikanStencilQuadInfo& quadInfo)
	: StencilComponentDefinition(
		quadInfo.stencil_id,
		quadInfo.parent_anchor_id,
		quadInfo.stencil_name.getValue(),
		quadInfo.relative_transform)
{
	m_quadWidth= quadInfo.quad_width;
	m_quadHeight= quadInfo.quad_height;
	m_bIsDoubleSided= quadInfo.is_double_sided;
}

configuru::Config QuadStencilDefinition::writeToJSON()
{
	configuru::Config pt = StencilComponentDefinition::writeToJSON();

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

MikanStencilQuadInfo QuadStencilDefinition::getQuadInfo() const
{
	const std::string& quadName = getComponentName();
	GlmTransform xform = getRelativeTransform();

	MikanStencilQuadInfo quadInfo= {};
	quadInfo.stencil_id = m_stencilId;
	quadInfo.parent_anchor_id = m_parentAnchorId;
	quadInfo.relative_transform = glm_transform_to_MikanTransform(getRelativeTransform());
	quadInfo.quad_width= m_quadWidth;
	quadInfo.quad_height= m_quadHeight;
	quadInfo.is_double_sided= m_bIsDoubleSided;
	quadInfo.is_disabled= m_bIsDisabled;
	quadInfo.stencil_name= quadName;

	return quadInfo;
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
	QuadStencilDefinitionPtr quadDefinition= getQuadStencilDefinition();

	if (!quadDefinition->getIsDisabled() &&
		StencilObjectSystem::getSystem()->getStencilSystemConfig()->getRenderStencilsFlag())
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

		drawTransformedQuad(xform, quadDefinition->getQuadWidth(), quadDefinition->getQuadHeight(), color);
		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", quadDefinition->getStencilId());
	}
}

void QuadStencilComponent::updateBoxColliderExtents()
{
	QuadStencilDefinitionPtr quadDefinition= getQuadStencilDefinition();
	BoxColliderComponentPtr boxCollider = m_boxCollider.lock();
	if (boxCollider)
	{
		boxCollider->setHalfExtents(glm::vec3(quadDefinition->getQuadWidth(), quadDefinition->getQuadHeight(), 0.01f) * 0.5f);
	}
}

// -- IPropertyInterface ----
void QuadStencilComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	StencilComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(QuadStencilDefinition::k_quadStencilWidthPropertyId);
	outPropertyNames.push_back(QuadStencilDefinition::k_quadStencilHeightPropertyId);
	outPropertyNames.push_back(QuadStencilDefinition::k_quadStencilDoubleSidedPropertyId);
}

bool QuadStencilComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (StencilComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == QuadStencilDefinition::k_quadStencilWidthPropertyId)
	{
		outDescriptor = {QuadStencilDefinition::k_quadStencilWidthPropertyId, ePropertyDataType::datatype_float, ePropertySemantic::size1d};
		return true;
	}
	else if (propertyName == QuadStencilDefinition::k_quadStencilHeightPropertyId)
	{
		outDescriptor = {QuadStencilDefinition::k_quadStencilHeightPropertyId, ePropertyDataType::datatype_float, ePropertySemantic::size1d};
		return true;
	}
	else if (propertyName == QuadStencilDefinition::k_quadStencilDoubleSidedPropertyId)
	{
		outDescriptor = {QuadStencilDefinition::k_quadStencilDoubleSidedPropertyId, ePropertyDataType::datatype_bool, ePropertySemantic::checkbox};
		return true;
	}

	return false;
}

bool QuadStencilComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (StencilComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == QuadStencilDefinition::k_quadStencilWidthPropertyId)
	{
		float width= getQuadStencilDefinition()->getQuadWidth();
		outValue = width;
		return true;
	}
	else if (propertyName == QuadStencilDefinition::k_quadStencilHeightPropertyId)
	{
		float height = getQuadStencilDefinition()->getQuadHeight();
		outValue = height;
		return true;
	}
	else if (propertyName == QuadStencilDefinition::k_quadStencilDoubleSidedPropertyId)
	{
		bool isDoubleSided = getQuadStencilDefinition()->getIsDoubleSided();
		outValue = isDoubleSided;
		return true;
	}

	return false;
}

bool QuadStencilComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (StencilComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == QuadStencilDefinition::k_quadStencilWidthPropertyId)
	{
		float width = inValue.Get<float>();

		getQuadStencilDefinition()->setQuadWidth(width);
		return true;
	}
	else if (propertyName == QuadStencilDefinition::k_quadStencilHeightPropertyId)
	{
		float height = inValue.Get<float>();

		getQuadStencilDefinition()->setQuadHeight(height);
		return true;
	}
	else if (propertyName == QuadStencilDefinition::k_quadStencilDoubleSidedPropertyId)
	{
		bool isDoubleSided = inValue.Get<bool>();

		getQuadStencilDefinition()->setIsDoubleSided(isDoubleSided);
		return true;
	}

	return false;
}