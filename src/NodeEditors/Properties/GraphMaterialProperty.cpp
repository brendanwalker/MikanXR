#include "GraphMaterialProperty.h"
#include "GlMaterial.h"
#include "GlVertexDefinition.h"
#include "Graphs/NodeGraph.h"
#include "MaterialAssetReference.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- GraphMaterialProperty -----
GraphMaterialProperty::GraphMaterialProperty()
	: GraphProperty()
{}

GraphMaterialProperty::GraphMaterialProperty(NodeGraphPtr ownerGraph)
	: GraphProperty(ownerGraph)
{}

void GraphMaterialProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	// Section 1: Basic info
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	bool isNodeOpened = ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_SpanAvailWidth);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);

	if (isNodeOpened)
	{
		// Name
		ImGui::Text("\t\tName");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		std::string name = m_materialResource->getName();
		ImGui::Text(name.c_str());
	}

	// Section 2: Shaders
	// Add Button
	float xPos = ImGui::GetCursorPosX();
	ImGui::SetCursorPosX(325);
	if (ImGui::SmallButton(ICON_FK_PLUS_CIRCLE "##set_material"))
	{
		//TODO
	}
}

// -- GraphMaterialPropertyFactory -----
GraphPropertyPtr GraphMaterialPropertyFactory::createProperty(
	const NodeEditorState* editorState,
	const std::string& name) const
{
	// TODO: See if we have a Material asset reference given from a drag and drop interaction
	MaterialAssetReferencePtr MaterialAssetRef = std::make_shared<MaterialAssetReference>();
	GlMaterialPtr MaterialResource = std::make_shared<GlMaterial>();

	GraphMaterialPropertyPtr MaterialProperty = m_ownerGraph->allocateTypedProperty<GraphMaterialProperty>(name);
	MaterialProperty->setMaterialAssetReference(MaterialAssetRef);
	MaterialProperty->setMaterialResource(MaterialResource);

	return MaterialProperty;
}