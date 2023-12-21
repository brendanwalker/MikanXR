#include "GraphModelProperty.h"
#include "GlRenderModelResource.h"
#include "GlVertexDefinition.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"
#include "ModelAssetReference.h"
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
GraphModelProperty::GraphModelProperty() 
	: GraphProperty() 
{
}

GraphModelProperty::GraphModelProperty(NodeGraphPtr ownerGraph) 
	: GraphProperty(ownerGraph)
{
}

bool GraphModelProperty::loadFromConfig(const GraphPropertyConfig& config)
{
	if (GraphProperty::loadFromConfig(config))
	{
		const auto& propConfig = static_cast<const GraphModelPropertyConfig&>(config);
		if (propConfig.assetRefIndex != -1)
		{
			auto assetRef = getOwnerGraph()->getAssetReferenceByIndex(propConfig.assetRefIndex);
			auto materialAssetRef = std::dynamic_pointer_cast<ModelAssetReference>(assetRef);
			if (materialAssetRef)
			{
				setModelAssetReference(materialAssetRef);
				return true;
			}
			else
			{
				MIKAN_LOG_ERROR("GraphModelProperty::loadFromConfig") 
					<< "Invalid model asset reference: " << propConfig.assetRefIndex;
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

void GraphModelProperty::saveToConfig(GraphPropertyConfig& config) const
{
	auto& propConfig = static_cast<GraphModelPropertyConfig&>(config);

	// Default asset ref to invalid
	propConfig.assetRefIndex = -1;

	// If we have a valid asset ref, look up the index in the graph
	if (m_modelAssetRef)
	{
		propConfig.assetRefIndex = getOwnerGraph()->getAssetReferenceIndex(m_modelAssetRef);

		if (propConfig.assetRefIndex == -1)
		{
			MIKAN_LOG_ERROR("GraphMaterialProperty::saveToConfig") 
				<< "Model property has orphaned asset reference: " 
				<< m_modelAssetRef->getAssetPath();
		}
	}

	GraphProperty::saveToConfig(config);
}

void GraphModelProperty::editorHandleDragDrop(const class NodeEditorState& editorState)
{
	auto modelNode =
		std::static_pointer_cast<ModelNode>(
			ModelNodeFactory(getOwnerGraph()).createNode(&editorState));

	// Set this as the source model property for the new node
	auto self = std::static_pointer_cast<GraphModelProperty>(shared_from_this());
	modelNode->setModelSource(self);
}

void GraphModelProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	// Section 1: Basic info
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	bool isNodeOpened = ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_SpanAvailWidth);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);

	if (isNodeOpened)
	{
		// Name
		ImGui::Text("\t\tName");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		std::string name = m_modelResource->getName();
		ImGui::Text(name.c_str());

		// TODO: Show model properties
	}
}

// -- GraphModelPropertyFactory -----
GraphPropertyPtr GraphModelPropertyFactory::createProperty(
	const NodeEditorState* editorState,
	const std::string& name) const
{
	// TODO: See if we have a model asset reference given from a drag and drop interaction
	ModelAssetReferencePtr modelAssetRef= std::make_shared<ModelAssetReference>();
	GlRenderModelResourcePtr modelResource = std::make_shared<GlRenderModelResource>();

	GraphModelPropertyPtr modelProperty= m_ownerGraph->addTypedProperty<GraphModelProperty>(name);
	modelProperty->setModelAssetReference(modelAssetRef);
	modelProperty->setModelResource(modelResource);

	return modelProperty;
}
