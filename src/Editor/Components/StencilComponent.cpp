#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "StencilComponent.h"
#include "StencilObjectSystem.h"
#include "TransformComponent.h"
#include "MikanObject.h"
#include "StringUtils.h"

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
	: TransformComponentDefinition(componentName, xform)
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
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_parentAnchorPropertyId));
	}
}

void StencilComponentDefinition::setIsDisabled(bool flag)
{
	if (m_bIsDisabled != flag)
	{
		m_bIsDisabled = flag;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_stencilDisabledPropertyId));
	}
}

void StencilComponentDefinition::setCullMode(eStencilCullMode mode)
{
	if (m_cullMode != mode)
	{
		m_cullMode= mode;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_stencilCullModePropertyId));
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
		AnchorComponentPtr anchor = AnchorObjectSystem::getSystem()->getSpatialAnchorById(newParentId);

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
void StencilComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	TransformComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(StencilComponentDefinition::k_stencilDisabledPropertyId);
	outPropertyNames.push_back(StencilComponentDefinition::k_parentAnchorPropertyId);
	outPropertyNames.push_back(StencilComponentDefinition::k_stencilCullModePropertyId);
}

bool StencilComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (TransformComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == StencilComponentDefinition::k_stencilDisabledPropertyId)
	{
		outDescriptor = {StencilComponentDefinition::k_stencilDisabledPropertyId, ePropertyDataType::datatype_bool, ePropertySemantic::checkbox};
		return true;
	}
	else if (propertyName == StencilComponentDefinition::k_parentAnchorPropertyId)
	{
		outDescriptor = {StencilComponentDefinition::k_parentAnchorPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::anchor_id};
		return true;
	}
	else if (propertyName == StencilComponentDefinition::k_stencilCullModePropertyId)
	{
		outDescriptor = {StencilComponentDefinition::k_stencilCullModePropertyId, ePropertyDataType::datatype_int, ePropertySemantic::stencilCullMode};
		return true;
	}

	return false;
}

bool StencilComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (TransformComponent::getPropertyValue(propertyName, outValue))
		return true;

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
		outValue= (int)getStencilComponentDefinition()->getCullMode();
		return true;
	}

	return false;
}

bool StencilComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (TransformComponent::setPropertyValue(propertyName, inValue))
		return true;

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
		eStencilCullMode cullMode= (eStencilCullMode)inValue.Get<int>();

		getStencilComponentDefinition()->setCullMode(cullMode);
		return true;
	}

	return false;
}

// -- IFunctionInterface ----
const std::string StencilComponent::k_deleteStencilFunctionId= "delete_stencil";

void StencilComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	TransformComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_deleteStencilFunctionId);
}

bool StencilComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (TransformComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == StencilComponent::k_deleteStencilFunctionId)
	{
		outDescriptor = {StencilComponent::k_deleteStencilFunctionId, "Delete Stencil"};
		return true;
	}

	return false;
}

bool StencilComponent::invokeFunction(const std::string& functionName)
{
	if (TransformComponent::invokeFunction(functionName))
		return true;

	if (functionName == StencilComponent::k_deleteStencilFunctionId)
	{
		deleteStencil();
	}

	return false;
}

void StencilComponent::deleteStencil()
{
	getOwnerObject()->deleteSelfConfig();
}