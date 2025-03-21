#include "GraphModelProperty.h"
#include "MikanRenderModelResource.h"
#include "IMkVertexDefinition.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"
#include "ModelAssetReference.h"
#include "NodeEditorUI.h"
#include "Nodes/ModelNode.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- GraphModelPropertyConfig -----
configuru::Config GraphModelPropertyConfig::writeToJSON()
{
	configuru::Config pt = GraphPropertyConfig::writeToJSON();

	pt["asset_ref_index"] = assetRefIndex;

	return pt;
}

void GraphModelPropertyConfig::readFromJSON(const configuru::Config& pt)
{
	assetRefIndex = pt.get_or<int>("asset_ref_index", -1);

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphModelProperty -----
bool GraphModelProperty::loadFromConfig(
	GraphPropertyConfigConstPtr propConfig,
	const NodeGraphConfig& graphConfig)
{
	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		const auto modelPropConfig = std::static_pointer_cast<const GraphModelPropertyConfig>(propConfig);
		if (modelPropConfig->assetRefIndex != -1)
		{
			auto assetRef = getOwnerGraph()->getAssetReferenceByIndex(modelPropConfig->assetRefIndex);
			auto materialAssetRef = std::dynamic_pointer_cast<ModelAssetReference>(assetRef);
			if (materialAssetRef)
			{
				setModelAssetReference(materialAssetRef);
				return true;
			}
			else
			{
				MIKAN_LOG_ERROR("GraphModelProperty::loadFromConfig") 
					<< "Invalid model asset reference: " << modelPropConfig->assetRefIndex;
				setModelAssetReference(ModelAssetReferencePtr());
			}
		}
		else
		{
			// Config says the property had an empty asset reference
			setModelAssetReference(ModelAssetReferencePtr());
			return true;
		}
	}

	return false;
}

void GraphModelProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto propConfig = std::static_pointer_cast<GraphModelPropertyConfig>(config);

	// Default asset ref to invalid
	propConfig->assetRefIndex = -1;

	// If we have a valid asset ref, look up the index in the graph
	if (m_modelAssetRef)
	{
		propConfig->assetRefIndex = getOwnerGraph()->getAssetReferenceIndex(m_modelAssetRef);

		if (propConfig->assetRefIndex == -1)
		{
			MIKAN_LOG_ERROR("GraphMaterialProperty::saveToConfig") 
				<< "Model property has orphaned asset reference: " 
				<< m_modelAssetRef->getAssetPath();
		}
	}

	GraphProperty::saveToConfig(config);
}

void GraphModelProperty::editorHandleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	auto modelNode = m_ownerGraph->createTypedNode<ModelNode>(editorState);

	// Set this as the source model property for the new node
	auto self = std::static_pointer_cast<GraphModelProperty>(shared_from_this());
	modelNode->setModelSource(self);
}

void GraphModelProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Model"))
	{
		// Name
		NodeEditorUI::DrawStaticTextProperty("Name", m_modelResource->getName());

		// Drag-Drop Handling
		auto modelAssetRef= 
			NodeEditorUI::receiveTypedDragDropPayload<ModelAssetReference>(
				ModelAssetReference::k_assetClassName);
		if (modelAssetRef)
		{
			setModelAssetReference(modelAssetRef);
		}
	}
}