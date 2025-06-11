#include "CompositorComponent.h"
#include "App.h"
#include "MainWindow.h"
#include "ProjectConfig.h"
#include "TransformComponent.h"
#include "SelectionComponent.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

#include "NodeGraphAssetReference.h"
#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- CompositorConfig -----
const std::string CompositorDefinition::k_compositorGraphPathPropertyId = "script_path";

CompositorDefinition::CompositorDefinition()
	: MikanComponentDefinition()
	, m_compositorId(INVALID_MIKAN_ID)
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
{
}

CompositorDefinition::CompositorDefinition(
	MikanCompositorID compositorId,
	const std::string& compositorName)
	: MikanComponentDefinition(compositorName)
	, m_compositorId(compositorId)
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
{}

configuru::Config CompositorDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["id"] = m_compositorId;
	if (m_nodeGraphAssetRef)
	{
		pt[k_compositorGraphPathPropertyId] = m_nodeGraphAssetRef->writeToJSON();
	}

	return pt;
}

void CompositorDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_compositorId = pt.get<int>("id");

	m_componentScriptAssetRefConfig = NodeGraphAssetReferenceFactory().allocateAssetReferenceConfig();
	if (pt.has_key(k_compositorGraphPathPropertyId))
	{
		m_componentScriptAssetRefConfig->readFromJSON(pt[k_compositorGraphPathPropertyId]);
	}
}

bool CompositorDefinition::hasCompositorGraphPath() const
{
	return !m_nodeGraphAssetRef->assetPath.empty();
}

const std::filesystem::path& CompositorDefinition::getCompositorGraphPath() const
{
	return m_nodeGraphAssetRef->assetPath;
}

void CompositorDefinition::setCompositorGraphPath(const std::filesystem::path& graphPath)
{
	if (graphPath != m_nodeGraphAssetRef->assetPath)
	{
		m_nodeGraphAssetRef->assetPath= graphPath.string();
		markDirty(ConfigPropertyChangeSet().addPropertyName(MikanComponentDefinition::k_componentScriptPathPropertyId));
	}
}

// -- CompositorComponent -----
CompositorComponent::CompositorComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
	m_bWantsCustomRender = true;
}

void CompositorComponent::init()
{
	MikanComponent::init();

}

// -- IPropertyInterface ----
void CompositorComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(CompositorDefinition::k_compositorGraphPathPropertyId);
}

bool CompositorComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (MikanComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		outDescriptor = {CompositorDefinition::k_compositorGraphPathPropertyId, ePropertyDataType::datatype_string, ePropertySemantic::filename};
		return true;
	}

	return false;
}

bool CompositorComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		Rml::String filepath = getCompositorDefinition()->getCompositorGraphPath().string();

		outValue = filepath;
		return true;
	}

	return false;
}

bool CompositorComponent::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyAttribute(propertyName, attributeName, outValue))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		if (attributeName == *k_PropertyAttributeFileBrowseTitle)
		{
			outValue = "Select a graph";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilter)
		{
			outValue = ".graph";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilterDesc)
		{
			outValue = "Graph Files (.graph)";
		}
	}

	return false;
}

bool CompositorComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (MikanComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		const Rml::String fileString = inValue.Get<Rml::String>();
		const std::filesystem::path filePath(fileString);

		getCompositorDefinition()->setCompositorGraphPath(filePath);
		return true;
	}

	return false;
}