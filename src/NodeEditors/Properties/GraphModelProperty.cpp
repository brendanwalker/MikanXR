#include "GraphModelProperty.h"
#include "GlRenderModelResource.h"
#include "GlVertexDefinition.h"
#include "Graphs/NodeGraph.h"
#include "ModelAssetReference.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- GraphModelProperty -----
GraphModelProperty::GraphModelProperty() 
	: GraphProperty() 
{
}

GraphModelProperty::GraphModelProperty(NodeGraphPtr ownerGraph) 
	: GraphProperty(ownerGraph)
{
}

void GraphModelProperty::editorHandleDragDrop(const class NodeEditorState& editorState)
{
	//TODO
	//auto modelNode =
	//	std::static_pointer_cast<ModelNode>(
	//		ModelNodeFactory(getOwnerGraph()).createNode(&editorState));

	//// Set this as the source model property for the new node
	//auto self = std::static_pointer_cast<GraphModelProperty>(shared_from_this());
	//modelNode->setTextureSource(self);
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
