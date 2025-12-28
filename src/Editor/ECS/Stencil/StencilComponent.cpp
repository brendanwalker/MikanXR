#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "StencilComponent.h"
#include "StencilObjectSystem.h"
#include "TransformComponent.h"
#include "MikanObject.h"
#include "StringUtils.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- StencilComponentConfig -----
const std::string StencilComponentDefinition::k_parentAnchorPropertyId = "parent_anchor_id";
const std::string StencilComponentDefinition::k_stencilDisabledPropertyId = "is_disabled";
const std::string StencilComponentDefinition::k_stencilCullModePropertyId = "cull_mode";

StencilComponentDefinition::StencilComponentDefinition()
	: m_stencilId(INVALID_MIKAN_ID)
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
	, m_stencilId(stencilId)
	, m_parentAnchorId(parentAnchorId)
	, m_bIsDisabled(false)
{
}

configuru::Config StencilComponentDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt["stencil_id"] = m_stencilId;
	pt[k_parentAnchorPropertyId] = m_parentAnchorId;
	pt[k_stencilDisabledPropertyId] = m_bIsDisabled;
	pt[k_stencilCullModePropertyId]= k_stencilCullModeStrings[(int)m_cullMode];

	return pt;
}

void StencilComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_stencilId = pt.get<int>("stencil_id");
	m_parentAnchorId = pt.get_or<int>(k_parentAnchorPropertyId, INVALID_MIKAN_ID);
	m_bIsDisabled = pt.get_or<bool>(k_stencilDisabledPropertyId, false);

	const std::string modeName= pt.get_or<std::string>(k_stencilCullModePropertyId, k_stencilCullModeStrings[0]);
	m_cullMode= StringUtils::FindEnumValue<eStencilCullMode>(modeName, k_stencilCullModeStrings);
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

// -- IRmlPropertyInterface ----
void StencilComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			StencilComponentDefinition::k_stencilDisabledPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			StencilComponentDefinition::k_parentAnchorPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			StencilComponentDefinition::k_stencilCullModePropertyId));
}

bool StencilComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

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

	return TransformComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool StencilComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == StencilComponentDefinition::k_stencilDisabledPropertyId)
	{
		bool bIsDisabled = inValue.Get<bool>();

		getStencilComponentDefinition()->setIsDisabled(bIsDisabled);
		return true;
	}
	else if (propertyName == StencilComponentDefinition::k_parentAnchorPropertyId)
	{
		MikanSpatialAnchorID anchorId = inValue.Get<int>();

		attachTransformComponentToAnchor(anchorId);
		return true;
	}
	else if (propertyName == StencilComponentDefinition::k_stencilCullModePropertyId)
	{
		eStencilCullMode cullMode = (eStencilCullMode)inValue.Get<int>();

		getStencilComponentDefinition()->setCullMode(cullMode);
		return true;
	}

	return TransformComponent::setPropertyValueFromRml(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
const std::string StencilComponent::k_deleteStencilFunctionId= "delete_stencil";

void StencilComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_deleteStencilFunctionId, "Delete Stencil"));
}

bool StencilComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	const std::string& id = functionDesc->getFunctionName();

	if (id == k_deleteStencilFunctionId)
	{
		deleteStencil();
		return true;
	}

	return TransformComponent::invokeFunctionFromRml(functionDesc);
}


void StencilComponent::deleteStencil()
{
	getOwnerObject()->deleteSelfConfig();
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