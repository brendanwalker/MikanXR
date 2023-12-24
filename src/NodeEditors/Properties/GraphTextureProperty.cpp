#include "GraphTextureProperty.h"
#include "GlTexture.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"
#include "Nodes/TextureNode.h"
#include "TextureAssetReference.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- GraphTexturePropertyConfig -----
configuru::Config GraphTexturePropertyConfig::writeToJSON()
{
	configuru::Config pt = GraphPropertyConfig::writeToJSON();

	pt["asset_ref_index"] = assetRefIndex;

	return pt;
}

void GraphTexturePropertyConfig::readFromJSON(const configuru::Config& pt)
{
	assetRefIndex = pt.get_or<int>("asset_ref_index", -1);

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphModelProperty -----
bool GraphTextureProperty::loadFromConfig(
	GraphPropertyConfigConstPtr propConfig,
	const NodeGraphConfig& graphConfig)
{
	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		const auto texturePropConfig = std::static_pointer_cast<const GraphTexturePropertyConfig>(propConfig);
		if (texturePropConfig->assetRefIndex != -1)
		{
			auto assetRef = getOwnerGraph()->getAssetReferenceByIndex(texturePropConfig->assetRefIndex);
			auto textureAssetRef = std::dynamic_pointer_cast<TextureAssetReference>(assetRef);
			if (textureAssetRef)
			{
				setTextureAssetReference(textureAssetRef);
				return true;
			}
			else
			{
				MIKAN_LOG_ERROR("GraphTextureProperty::loadFromConfig") 
					<< "Invalid texture asset reference: " << texturePropConfig->assetRefIndex;
				setTextureAssetReference(TextureAssetReferencePtr());
			}
		}
		else
		{
			// Config says the property had an empty asset reference
			setTextureAssetReference(TextureAssetReferencePtr());
			return true;
		}
	}

	return false;
}

void GraphTextureProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto propConfig = std::static_pointer_cast<GraphTexturePropertyConfig>(config);

	// Default asset ref to invalid
	propConfig->assetRefIndex = -1;

	// If we have a valid asset ref, look up the index in the graph
	if (m_textureAssetRef)
	{
		propConfig->assetRefIndex = getOwnerGraph()->getAssetReferenceIndex(m_textureAssetRef);

		if (propConfig->assetRefIndex == -1)
		{
			MIKAN_LOG_ERROR("GraphTextureProperty::saveToConfig") 
				<< "Texture property has orphaned asset reference: " 
				<< m_textureAssetRef->getAssetPath();
		}
	}

	GraphProperty::saveToConfig(config);
}

void GraphTextureProperty::editorHandleDragDrop(const class NodeEditorState& editorState)
{
	auto textureNode =
		std::static_pointer_cast<TextureNode>(
			TextureNodeFactory().createNode(editorState));

	// Set this as the source texture property for the new node
	auto self= std::static_pointer_cast<GraphTextureProperty>(shared_from_this());
	textureNode->setTextureSource(self);
}

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

		// TODO: Show Texture properties
	}
}