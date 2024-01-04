#include "GraphTextureProperty.h"
#include "GlTexture.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"
#include "NodeEditorUI.h"
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

void GraphTextureProperty::editorHandleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	auto textureNode = m_ownerGraph->createTypedNode<TextureNode>(editorState);

	// Set this as the source texture property for the new node
	auto self= std::static_pointer_cast<GraphTextureProperty>(shared_from_this());
	textureNode->setTextureSource(self);
}

void GraphTextureProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Texture"))
	{
		// Name
		std::string name = m_texture ? m_texture->getName() : "<No Texture>";
		NodeEditorUI::DrawStaticTextProperty("Name", name);

		// Drag-Drop Handling
		auto textureAssetRef =
			NodeEditorUI::receiveTypedDragDropPayload<TextureAssetReference>(
				TextureAssetReference::k_assetClassName);
		if (textureAssetRef)
		{
			setTextureAssetReference(textureAssetRef);
		}
	}
}