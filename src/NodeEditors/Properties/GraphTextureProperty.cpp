#include "GraphTextureProperty.h"
#include "GlTexture.h"
#include "Graphs/NodeGraph.h"
#include "TextureAssetReference.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- GraphModelProperty -----
GraphTextureProperty::GraphTextureProperty()
	: GraphProperty()
{}

GraphTextureProperty::GraphTextureProperty(NodeGraphPtr ownerGraph)
	: GraphProperty(ownerGraph)
{}

void GraphTextureProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	// Section 1: Basic info
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	bool isNodeOpened = ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_SpanAvailWidth);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);

	if (isNodeOpened)
	{
		// Name
		ImGui::Text("\t\tName");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		std::string name = m_texture->getName();
		ImGui::Text(name.c_str());
	}

	// Section 2: Shaders
	// Add Button
	float xPos = ImGui::GetCursorPosX();
	ImGui::SetCursorPosX(325);
	if (ImGui::SmallButton(ICON_FK_PLUS_CIRCLE "##set_texture"))
	{
	}
}

// -- GraphModelPropertyFactory -----
GraphPropertyPtr GraphTexturePropertyFactory::createProperty(
	const NodeEditorState* editorState,
	const std::string& name) const
{
	// TODO: See if we have a texture asset reference given from a drag and drop interaction
	TextureAssetReferencePtr textureAssetRef = std::make_shared<TextureAssetReference>();
	GlTexturePtr textureResource = std::make_shared<GlTexture>();

	auto textureProperty = m_ownerGraph->allocateTypedProperty<GraphTextureProperty>(name);
	textureProperty->setTextureAssetReference(textureAssetRef);
	textureProperty->setTextureResource(textureResource);

	return textureProperty;
}
