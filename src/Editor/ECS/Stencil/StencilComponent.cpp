#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "StencilComponent.h"
#include "TransformComponent.h"
#include "MikanObject.h"
#include "MikanStencilTypes.h"
#include "StringUtils.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- StencilComponentConfig -----
const std::string StencilComponentDefinition::k_parentAnchorPropertyId = "parent_anchor_id";
const std::string StencilComponentDefinition::k_stencilDisabledPropertyId = "is_disabled";
const std::string StencilComponentDefinition::k_stencilCullModePropertyId = "cull_mode";

StencilComponentDefinition::StencilComponentDefinition()
	: TransformComponentDefinition()
	, m_parentAnchorId(INVALID_MIKAN_ID)
	, m_bIsDisabled(false)
{
}

StencilComponentDefinition::StencilComponentDefinition(
	MikanStencilID stencilId,
	MikanSpatialAnchorID parentAnchorId,
	const std::string& componentName, 
	const MikanTransform& xform)
	: TransformComponentDefinition(stencilId, componentName, xform)
	, m_parentAnchorId(parentAnchorId)
	, m_bIsDisabled(false)
{
}

configuru::Config StencilComponentDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt[k_parentAnchorPropertyId] = m_parentAnchorId;
	pt[k_stencilDisabledPropertyId] = m_bIsDisabled;
	pt[k_stencilCullModePropertyId]= k_stencilCullModeStrings[(int)m_cullMode];

	return pt;
}

void StencilComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_parentAnchorId = pt.get_or<int>(k_parentAnchorPropertyId, INVALID_MIKAN_ID);
	m_bIsDisabled = pt.get_or<bool>(k_stencilDisabledPropertyId, false);

	const std::string modeName= pt.get_or<std::string>(k_stencilCullModePropertyId, k_stencilCullModeStrings[0]);
	m_cullMode= StringUtils::FindEnumValue<eStencilCullMode>(modeName, k_stencilCullModeStrings);
}

bool StencilComponentDefinition::readFromInitParams(const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!TransformComponentDefinition::readFromInitParams(initParams))
		return false;

	const auto* componentValues = initParams.getTypedPointer<MikanStencilComponentValues>();
	if (componentValues)
	{
		m_parentAnchorId = componentValues->parent_anchor_id;
		m_bIsDisabled = componentValues->is_disabled;
		m_cullMode = (eStencilCullMode)componentValues->cull_mode;
	}

	return true;
}

void StencilComponentDefinition::setParentAnchorId(MikanSpatialAnchorID anchorId)
{
	if (m_parentAnchorId != anchorId)
	{
		m_parentAnchorId = anchorId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_parentAnchorPropertyId));
	}
}

void StencilComponentDefinition::setIsDisabled(bool flag)
{
	if (m_bIsDisabled != flag)
	{
		m_bIsDisabled = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_stencilDisabledPropertyId));
	}
}

void StencilComponentDefinition::setCullMode(eStencilCullMode mode)
{
	if (m_cullMode != mode)
	{
		m_cullMode= mode;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_stencilCullModePropertyId));
	}
}

// -- StencilComponent -----
StencilComponent::StencilComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
}

// -- IEntityAccessor ----
rfk::Struct const* StencilComponent::getClientAPIValuesStructType() const
{
	return &MikanStencilComponentValues::staticGetArchetype();
}

void StencilComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	TransformComponent::setDefinition(definition);

	// Setup initial attachment
	auto stencilComponentConfigPtr = std::static_pointer_cast<StencilComponentDefinition>(definition);
	MikanSpatialAnchorID currentParentId = stencilComponentConfigPtr->getParentAnchorId();
	attachTransformComponentToAnchor(currentParentId);
}

void StencilComponent::attachTransformComponentToAnchor(MikanSpatialAnchorID newParentId)
{
	if (newParentId != INVALID_MIKAN_ID)
	{
		AnchorComponentPtr anchor = getObjectSystemOfType<AnchorObjectSystem>()->getSpatialAnchorById(newParentId);

		if (anchor)
		{
			if (attachToComponent(anchor->getOwnerObject()->getRootComponent()))
			{
				getStencilComponentDefinition()->setParentAnchorId(newParentId);
			}
		}
		else
		{
			detachFromParent(eDetachReason::detachFromParent);
			getStencilComponentDefinition()->setParentAnchorId(INVALID_MIKAN_ID);
		}
	}
	else
	{
		detachFromParent(eDetachReason::detachFromParent);
		getStencilComponentDefinition()->setParentAnchorId(INVALID_MIKAN_ID);
	}
}

// -- IPropertyInterface ----
void StencilComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			StencilComponentDefinition::k_stencilDisabledPropertyId, MikanVariantType::BOOL));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			StencilComponentDefinition::k_parentAnchorPropertyId, MikanVariantType::INT)
		->setDefaultValue(-1));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			StencilComponentDefinition::k_stencilCullModePropertyId, MikanVariantType::INT)
		->setDefaultValue((int)eStencilCullMode::none));
}

bool StencilComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == StencilComponentDefinition::k_stencilDisabledPropertyId)
	{
		outValue = getStencilComponentDefinition()->getIsDisabled();
		return true;
	}
	else if (propertyName == StencilComponentDefinition::k_parentAnchorPropertyId)
	{
		outValue = getStencilComponentDefinition()->getParentAnchorId();
		return true;
	}
	else if (propertyName == StencilComponentDefinition::k_stencilCullModePropertyId)
	{
		outValue = (int)getStencilComponentDefinition()->getCullMode();
		return true;
	}

	return TransformComponent::getPropertyValue(propertyName, outValue);
}

bool StencilComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == StencilComponentDefinition::k_stencilDisabledPropertyId)
	{
		bool bIsDisabled = inValue.getBoolValue();

		getStencilComponentDefinition()->setIsDisabled(bIsDisabled);
		return true;
	}
	else if (propertyName == StencilComponentDefinition::k_parentAnchorPropertyId)
	{
		MikanSpatialAnchorID anchorId = inValue.getIntValue();

		attachTransformComponentToAnchor(anchorId);
		return true;
	}
	else if (propertyName == StencilComponentDefinition::k_stencilCullModePropertyId)
	{
		eStencilCullMode cullMode = (eStencilCullMode)inValue.getIntValue();

		getStencilComponentDefinition()->setCullMode(cullMode);
		return true;
	}

	return TransformComponent::setPropertyValue(propertyName, inValue);
}

// -- Lua Binding ----
void StencilComponent::bindLuaFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<StencilComponent, TransformComponent>("StencilComponent")
		.addProperty("parentAnchorId",
			[](StencilComponent* component) -> int {
				return component->getStencilComponentDefinition()->getParentAnchorId();
			})
		//TODO
		//.addProperty("parentAnchor",
		//	[](SceneComponent* component) -> AnchorComponent* {
		//		return component->getParentAnchor().get();
		//	})
		.addProperty("isDisabled",
			[](StencilComponent* component) -> bool {
				return component->getStencilComponentDefinition()->getIsDisabled();
			},
			[](StencilComponent* component, bool isDisabled) {
				component->getStencilComponentDefinition()->setIsDisabled(isDisabled);
			})
		.addProperty("cullMode",
			[](StencilComponent* component) -> eStencilCullMode {
				return component->getStencilComponentDefinition()->getCullMode();
			},
			[](StencilComponent* component, eStencilCullMode cullMode) {
				component->getStencilComponentDefinition()->setCullMode(cullMode);
			})
		.endClass();
}