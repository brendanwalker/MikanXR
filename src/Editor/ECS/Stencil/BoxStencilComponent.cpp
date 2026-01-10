#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "Colors.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "AnchorComponent.h"
#include "TransformComponent.h"
#include "BoxColliderComponent.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "SelectionComponent.h"
#include "BoxStencilSystem.h"
#include "StringUtils.h"
#include "TextStyle.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

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
	const MikanStencilBoxInfo& boxInfo)
	: StencilComponentDefinition(
		boxInfo.stencil_id,
		boxInfo.parent_anchor_id,
		boxInfo.stencil_name.getValue(),
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

MikanStencilBoxInfo BoxStencilDefinition::getBoxInfo() const
{
	const std::string& boxName = getComponentName();
	GlmTransform xform = getRelativeTransform();

	MikanStencilBoxInfo boxInfo= {};
	boxInfo.stencil_id = m_stencilId;
	boxInfo.parent_anchor_id = m_parentAnchorId;
	boxInfo.relative_transform = glm_transform_to_MikanTransform(getRelativeTransform());
	boxInfo.box_x_size = m_boxSize.x;
	boxInfo.box_y_size = m_boxSize.y;
	boxInfo.box_z_size = m_boxSize.z;
	boxInfo.is_disabled = m_bIsDisabled;
	boxInfo.stencil_name= boxName;

	return boxInfo;
}

void BoxStencilDefinition::setBoxXSize(float size)
{
	m_boxSize.x = size;
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_boxStencilXSizePropertyId));
}

void BoxStencilDefinition::setBoxYSize(float size)
{
	m_boxSize.y = size;
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_boxStencilYSizePropertyId));
}

void BoxStencilDefinition::setBoxZSize(float size)
{
	m_boxSize.z = size;
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_boxStencilZSizePropertyId));
}

void BoxStencilDefinition::setBoxSize(float xSize, float ySize, float zSize)
{
	m_boxSize.x = xSize;
	m_boxSize.y = ySize;
	m_boxSize.z = zSize;
	notifyPropertyChanged(ConfigPropertyChangeSet()
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
	BoxStencilDefinitionPtr boxDefinition= getBoxStencilDefinition();
	BoxStencilSystemPtr boxStencilSystem = getObjectSystemOfType<BoxStencilSystem>();

	if (!boxDefinition->getIsDisabled() &&
		boxStencilSystem && boxStencilSystem->getBoxStencilSystemDefinitionConst()->getRenderStencilsFlag())
	{
		TextStyle style = getDefaultTextStyle();

		const float xSize= boxDefinition->getBoxXSize();
		const float ySize= boxDefinition->getBoxYSize();
		const float zSize= boxDefinition->getBoxZSize();
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
		drawTextAtWorldPosition(style, position, L"Stencil %d", boxDefinition->getStencilId());
	}
}

void BoxStencilComponent::updateBoxColliderExtents()
{
	BoxStencilDefinitionPtr boxDefinition= getBoxStencilDefinition();
	BoxColliderComponentPtr boxCollider= m_boxCollider.lock();

	if (boxCollider && boxDefinition)
	{
		const float xSize = boxDefinition->getBoxXSize();
		const float ySize = boxDefinition->getBoxYSize();
		const float zSize = boxDefinition->getBoxZSize();

		boxCollider->setHalfExtents(glm::vec3(xSize, ySize, zSize) * 0.5f);
	}
}

// -- IRmlPropertyInterface ----
void BoxStencilComponent::getRmlPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	StencilComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			BoxStencilDefinition::k_boxStencilXSizePropertyId, MikanVariantType::FLOAT));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			BoxStencilDefinition::k_boxStencilYSizePropertyId, MikanVariantType::FLOAT));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			BoxStencilDefinition::k_boxStencilZSizePropertyId, MikanVariantType::FLOAT));
}

bool BoxStencilComponent::getPropertyValue(
	PropertyDescriptorConstPtr propertyDesc,
	MikanVariant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == BoxStencilDefinition::k_boxStencilXSizePropertyId)
	{
		outValue = getBoxStencilDefinition()->getBoxXSize();
		return true;
	}
	else if (propertyName == BoxStencilDefinition::k_boxStencilYSizePropertyId)
	{
		outValue = getBoxStencilDefinition()->getBoxYSize();
		return true;
	}
	else if (propertyName == BoxStencilDefinition::k_boxStencilZSizePropertyId)
	{
		outValue = getBoxStencilDefinition()->getBoxZSize();
		return true;
	}

	return StencilComponent::getPropertyValue(propertyDesc, outValue);
}

bool BoxStencilComponent::setPropertyValue(
	PropertyDescriptorConstPtr propertyDesc,
	const MikanVariant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == BoxStencilDefinition::k_boxStencilXSizePropertyId)
	{
		float xSize = inValue.getFloatValue();

		getBoxStencilDefinition()->setBoxXSize(xSize);
		return true;
	}
	else if (propertyName == BoxStencilDefinition::k_boxStencilYSizePropertyId)
	{
		float ySize = inValue.getFloatValue();

		getBoxStencilDefinition()->setBoxYSize(ySize);
		return true;
	}
	else if (propertyName == BoxStencilDefinition::k_boxStencilZSizePropertyId)
	{
		float zSize = inValue.getFloatValue();

		getBoxStencilDefinition()->setBoxZSize(zSize);
		return true;
	}

	return StencilComponent::setPropertyValue(propertyDesc, inValue);
}

// -- Lua Binding ----
void BoxStencilComponent::bindLuaFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<BoxStencilComponent, StencilComponent>("BoxStencilComponent")
		.addProperty("boxXSize",
			[](BoxStencilComponent* component) -> float {
				return component->getBoxStencilDefinition()->getBoxXSize();
			},
			[](BoxStencilComponent* component, float value) {
				component->getBoxStencilDefinition()->setBoxXSize(value);
			})
		.addProperty("boxYSize",
			[](BoxStencilComponent* component) -> float {
				return component->getBoxStencilDefinition()->getBoxYSize();
			},
			[](BoxStencilComponent* component, float value) {
				component->getBoxStencilDefinition()->setBoxYSize(value);
			})
		.addProperty("boxZSize",
			[](BoxStencilComponent* component) -> float {
				return component->getBoxStencilDefinition()->getBoxZSize();
			},
			[](BoxStencilComponent* component, float value) {
				component->getBoxStencilDefinition()->setBoxZSize(value);
			})
		.endClass();
}