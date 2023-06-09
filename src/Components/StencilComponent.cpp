#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "StencilComponent.h"
#include "StencilObjectSystem.h"
#include "SceneComponent.h"
#include "MikanObject.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- StencilComponentConfig -----
const std::string StencilComponentDefinition::k_parentAnchorPropertyId = "parent_anchor_id";
const std::string StencilComponentDefinition::k_stencilDisabledPropertyId = "is_disabled";

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
	: SceneComponentDefinition(componentName, xform)
	, m_stencilId(stencilId)
	, m_parentAnchorId(parentAnchorId)
	, m_bIsDisabled(false)
{
}

configuru::Config StencilComponentDefinition::writeToJSON()
{
	configuru::Config pt = SceneComponentDefinition::writeToJSON();

	pt["stencil_id"] = m_stencilId;
	pt["parent_anchor_id"] = m_parentAnchorId;
	pt["is_disabled"] = m_bIsDisabled;

	return pt;
}

void StencilComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	SceneComponentDefinition::readFromJSON(pt);

	m_stencilId = pt.get<int>("stencil_id");
	m_parentAnchorId = pt.get_or<int>("parent_anchor_id", INVALID_MIKAN_ID);
	m_bIsDisabled = pt.get_or<bool>("is_disabled", false);
}

void StencilComponentDefinition::setParentAnchorId(MikanSpatialAnchorID anchorId)
{
	m_parentAnchorId = anchorId;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_parentAnchorPropertyId));
}

void StencilComponentDefinition::setIsDisabled(bool flag)
{
	m_bIsDisabled = flag;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_stencilDisabledPropertyId));
}

// -- StencilComponent -----
StencilComponent::StencilComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
}

void StencilComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	SceneComponent::setDefinition(definition);

	// Setup initial attachment
	auto stencilComponentConfigPtr = std::static_pointer_cast<StencilComponentDefinition>(definition);
	MikanSpatialAnchorID currentParentId = stencilComponentConfigPtr->getParentAnchorId();
	attachSceneComponentToAnchor(currentParentId);
}

void StencilComponent::attachSceneComponentToAnchor(MikanSpatialAnchorID newParentId)
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
	SceneComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(StencilComponentDefinition::k_stencilDisabledPropertyId);
	outPropertyNames.push_back(StencilComponentDefinition::k_parentAnchorPropertyId);
}

bool StencilComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (SceneComponent::getPropertyDescriptor(propertyName, outDescriptor))
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

	return false;
}

bool StencilComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (SceneComponent::getPropertyValue(propertyName, outValue))
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


	return false;
}

bool StencilComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (SceneComponent::setPropertyValue(propertyName, inValue))
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

		attachSceneComponentToAnchor(anchorId);
		return true;
	}

	return false;
}

// -- IFunctionInterface ----
const std::string StencilComponent::k_deleteStencilFunctionId= "delete_stencil";

void StencilComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	SceneComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_deleteStencilFunctionId);
}

bool StencilComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (SceneComponent::getFunctionDescriptor(functionName, outDescriptor))
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
	if (SceneComponent::invokeFunction(functionName))
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