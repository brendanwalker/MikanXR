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
	}

	// Section 2: Shaders
	// Add Button
	float xPos = ImGui::GetCursorPosX();
	ImGui::SetCursorPosX(325);
	if (ImGui::SmallButton(ICON_FK_PLUS_CIRCLE "##set_mesh"))
	{
	//TODO
	#if 0
		auto paths_c = tinyfd_openFileDialog("Add Shader", "", 0, 0, 0, 1);
		if (paths_c)
		{
			std::stringstream ssPaths(paths_c);
			std::string path;
			while (std::getline(ssPaths, path, '|'))
				m_Programs[m_SelectedItemId]->AddShader(
					PathUtils::makeUniversalPathString(path).c_str(),
					GL_VERTEX_SHADER);
		}
	#endif
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

	GraphModelPropertyPtr modelProperty= m_ownerGraph->allocateTypedProperty<GraphModelProperty>(name);
	modelProperty->setModelAssetReference(modelAssetRef);
	modelProperty->setModelResource(modelResource);

	return modelProperty;
}
